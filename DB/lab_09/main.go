package main

import (
	"context"
	"database/sql"
	"encoding/csv"
	"encoding/json"
	"flag"
	"fmt"
	"log"
	"math/rand"
	"os"
	"time"

	_ "github.com/lib/pq"
	"github.com/redis/go-redis/v9"
)

const cacheKey = "stats:top_books"

type BookStat struct {
	BookID int    `json:"book_id"`
	Title  string `json:"title"`
	Loans  int    `json:"loans"`
}

func main() {
	var (
		pgDSN           = flag.String("pg", "postgres://postgres:postgres@localhost:5432/postgres?sslmode=disable", "Postgres DSN")
		redisAdr        = flag.String("redis", "localhost:5433", "Redis host:port (у тебя 5433:6379)")
		mode            = flag.String("mode", "none", "none|insert|delete|update")
		duration        = flag.Duration("duration", 2*time.Minute, "How long to run (e.g. 2m, 30s)")
		outCSV          = flag.String("out", "results.csv", "Output CSV file")
		measureEvery    = flag.Duration("measure_every", 5*time.Second, "Measure interval")
		mutateEvery     = flag.Duration("mutate_every", 10*time.Second, "Mutation interval (insert/delete/update)")
		cacheTTL        = flag.Duration("cache_ttl", 60*time.Second, "Redis cache TTL")
		logMutations    = flag.Bool("log_mutations", true, "Log mutation ticks")
		fillCacheOnMiss = flag.Bool("fill_cache_on_miss", true, "Fill cache when miss using already computed DB result")
	)
	flag.Parse()

	ctx := context.Background()
	runCtx, cancel := context.WithTimeout(ctx, *duration)
	defer cancel()

	db, err := sql.Open("postgres", *pgDSN)
	if err != nil {
		log.Fatalf("open postgres: %v", err)
	}
	defer db.Close()
	if err := db.PingContext(runCtx); err != nil {
		log.Fatalf("ping postgres: %v", err)
	}

	rdb := redis.NewClient(&redis.Options{Addr: *redisAdr})
	defer rdb.Close()
	if err := rdb.Ping(runCtx).Err(); err != nil {
		log.Fatalf("ping redis: %v", err)
	}

	if err := seedIfEmpty(runCtx, db); err != nil {
		log.Fatalf("seed: %v", err)
	}

	f, err := os.Create(*outCSV)
	if err != nil {
		log.Fatalf("create csv: %v", err)
	}
	defer f.Close()

	w := csv.NewWriter(f)
	defer w.Flush()

	_ = w.Write([]string{"ts", "mode", "db_ms", "redis_ms", "cache_hit"})

	if *mode != "none" {
		go func() {
			t := time.NewTicker(*mutateEvery)
			defer t.Stop()
			for {
				select {
				case <-t.C:
					if *logMutations {
						log.Printf("[MUTATE] mode=%s at %s", *mode, time.Now().Format(time.RFC3339Nano))
					}
					if err := mutate(runCtx, db, rdb, *mode); err != nil {
						log.Printf("mutate(%s) error: %v", *mode, err)
					}
				case <-runCtx.Done():
					return
				}
			}
		}()
	}

	ticker := time.NewTicker(*measureEvery)
	defer ticker.Stop()

	for {
		select {
		case <-ticker.C:
			dbStart := time.Now()
			stats, err := queryTopBooks(runCtx, db)
			dbDur := time.Since(dbStart)
			if err != nil {
				log.Printf("db query error: %v", err)
				continue
			}

			redisDur, hit, err := measureRedisGet(runCtx, rdb)
			if err != nil {
				log.Printf("redis get error: %v", err)
			}

			if !hit && *fillCacheOnMiss {
				_ = setCache(runCtx, rdb, stats, *cacheTTL)
			}

			ts := time.Now().Format(time.RFC3339Nano)
			_ = w.Write([]string{
				ts,
				*mode,
				fmt.Sprintf("%.3f", float64(dbDur.Microseconds())/1000.0),
				fmt.Sprintf("%.3f", float64(redisDur.Microseconds())/1000.0),
				fmt.Sprintf("%t", hit),
			})
			w.Flush()

			log.Printf("[%s] db=%v redis_get=%v hit=%v", *mode, dbDur, redisDur, hit)

		case <-runCtx.Done():
			log.Printf("done, wrote %s", *outCSV)
			return
		}
	}
}

func queryTopBooks(ctx context.Context, db *sql.DB) ([]BookStat, error) {
	const q = `
SELECT b.id, b.title, COUNT(*) AS loans_count
FROM tp.book_loans bl
JOIN tp.books b ON b.id = bl.book_id
GROUP BY b.id, b.title
ORDER BY loans_count DESC
LIMIT 10;`

	rows, err := db.QueryContext(ctx, q)
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	var out []BookStat
	for rows.Next() {
		var s BookStat
		if err := rows.Scan(&s.BookID, &s.Title, &s.Loans); err != nil {
			return nil, err
		}
		out = append(out, s)
	}
	return out, rows.Err()
}

func measureRedisGet(ctx context.Context, rdb *redis.Client) (time.Duration, bool, error) {
	start := time.Now()
	b, err := rdb.Get(ctx, cacheKey).Bytes()
	dur := time.Since(start)

	if err == redis.Nil {
		return dur, false, nil
	}
	if err != nil {
		return dur, false, err
	}

	var cached []BookStat
	if err := json.Unmarshal(b, &cached); err != nil {
		return dur, false, nil
	}
	return dur, true, nil
}

func setCache(ctx context.Context, rdb *redis.Client, stats []BookStat, ttl time.Duration) error {
	b, err := json.Marshal(stats)
	if err != nil {
		return err
	}
	return rdb.Set(ctx, cacheKey, b, ttl).Err()
}

func mutate(ctx context.Context, db *sql.DB, rdb *redis.Client, mode string) error {
	switch mode {
	case "insert":
		if err := insertLoan(ctx, db); err != nil {
			return err
		}
	case "delete":
		if err := deleteOneLoan(ctx, db); err != nil {
			return err
		}
	case "update":
		if err := updateOneLoan(ctx, db); err != nil {
			return err
		}
	default:
		return nil
	}

	_ = rdb.Del(ctx, cacheKey).Err()
	return nil
}

func nextID(ctx context.Context, db *sql.DB, table string) (int, error) {
	q := fmt.Sprintf(`SELECT COALESCE(MAX(id), 0) + 1 FROM %s;`, table)
	var id int
	if err := db.QueryRowContext(ctx, q).Scan(&id); err != nil {
		return 0, err
	}
	return id, nil
}

func pickRandomIDOffset(ctx context.Context, db *sql.DB, table string) (int, error) {
	q := fmt.Sprintf(`
WITH c AS (SELECT COUNT(*)::int AS n FROM %s)
SELECT id
FROM %s
OFFSET (SELECT CASE WHEN n=0 THEN 0 ELSE floor(random()*n)::int END FROM c)
LIMIT 1;`, table, table)

	var id int
	if err := db.QueryRowContext(ctx, q).Scan(&id); err != nil {
		return 0, err
	}
	return id, nil
}

func insertLoan(ctx context.Context, db *sql.DB) error {
	loanID, err := nextID(ctx, db, "tp.book_loans")
	if err != nil {
		return err
	}
	bookID, err := pickRandomIDOffset(ctx, db, "tp.books")
	if err != nil {
		return err
	}
	authorID, err := pickRandomIDOffset(ctx, db, "tp.authors")
	if err != nil {
		return err
	}
	readerID, err := pickRandomIDOffset(ctx, db, "tp.readers")
	if err != nil {
		return err
	}

	loanDate := time.Now().Truncate(24 * time.Hour)
	dueDate := loanDate.AddDate(0, 0, 14)

	_, err = db.ExecContext(ctx, `
INSERT INTO tp.book_loans(id, book_id, author_id, reader_id, loan_date, due_date, return_date, status)
VALUES($1,$2,$3,$4,$5,$6,NULL,'Выдана');`,
		loanID, bookID, authorID, readerID, loanDate, dueDate,
	)
	return err
}

func deleteOneLoan(ctx context.Context, db *sql.DB) error {
	var id int
	err := db.QueryRowContext(ctx, `SELECT id FROM tp.book_loans ORDER BY id DESC LIMIT 1;`).Scan(&id)
	if err == sql.ErrNoRows {
		return nil
	}
	if err != nil {
		return err
	}
	_, err = db.ExecContext(ctx, `DELETE FROM tp.book_loans WHERE id=$1;`, id)
	return err
}

func updateOneLoan(ctx context.Context, db *sql.DB) error {
	var id int
	err := db.QueryRowContext(ctx, `
SELECT id
FROM tp.book_loans
WHERE status = 'Выдана'
ORDER BY id DESC
LIMIT 1;`).Scan(&id)

	if err == sql.ErrNoRows {
		err = db.QueryRowContext(ctx, `SELECT id FROM tp.book_loans ORDER BY id DESC LIMIT 1;`).Scan(&id)
		if err == sql.ErrNoRows {
			return nil
		}
	}
	if err != nil {
		return err
	}

	returnDate := time.Now().Truncate(24 * time.Hour)
	_, err = db.ExecContext(ctx, `
UPDATE tp.book_loans
SET status='Возвращена', return_date=$2
WHERE id=$1;`, id, returnDate)
	return err
}

func seedIfEmpty(ctx context.Context, db *sql.DB) error {
	var cnt int
	if err := db.QueryRowContext(ctx, `SELECT COUNT(*) FROM tp.genres;`).Scan(&cnt); err != nil {
		return err
	}
	if cnt > 0 {
		return nil
	}

	log.Printf("tables look empty -> seeding minimal data...")

	_, err := db.ExecContext(ctx, `
INSERT INTO tp.genres(id,name) VALUES
(1,'Фантастика'),(2,'Детектив'),(3,'Научпоп');`)
	if err != nil {
		return err
	}

	_, err = db.ExecContext(ctx, `
INSERT INTO tp.authors(id,first_name,last_name,birth_date,country,biography) VALUES
(1,'Айзек','Азимов','1920-01-02','USA',''),
(2,'Артур','Конан Дойл','1859-05-22','UK',''),
(3,'Стивен','Хокинг','1942-01-08','UK','');`)
	if err != nil {
		return err
	}

	_, err = db.ExecContext(ctx, `
INSERT INTO tp.books(id,title,genre_id,publication_year,publisher,page_count) VALUES
(1,'Основание',1,1951,'Gnome Press',255),
(2,'Шерлок Холмс',2,1892,'George Newnes',307),
(3,'Краткая история времени',3,1988,'Bantam Dell',256);`)
	if err != nil {
		return err
	}

	_, err = db.ExecContext(ctx, `
INSERT INTO tp.readers(id,surname,name,middle_name,address,sex,birthday,email,phone,registration_date) VALUES
(1,'Иванов','Иван','Иванович','Москва','М','2000-01-01','ivanov@example.com','+70000000001',CURRENT_DATE),
(2,'Петрова','Анна','Сергеевна','СПб','Ж','1999-02-02','petrova@example.com','+70000000002',CURRENT_DATE);`)
	if err != nil {
		return err
	}

	rand.Seed(time.Now().UnixNano())
	for i := 1; i <= 200; i++ {
		bookID := 1 + rand.Intn(3)
		authorID := 1 + rand.Intn(3)
		readerID := 1 + rand.Intn(2)

		loanDate := time.Now().AddDate(0, 0, -rand.Intn(60)).Truncate(24 * time.Hour)
		dueDate := loanDate.AddDate(0, 0, 14)

		_, err := db.ExecContext(ctx, `
INSERT INTO tp.book_loans(id, book_id, author_id, reader_id, loan_date, due_date, return_date, status)
VALUES($1,$2,$3,$4,$5,$6,NULL,'Выдана');`,
			i, bookID, authorID, readerID, loanDate, dueDate,
		)
		if err != nil {
			return err
		}
	}

	log.Printf("seed done")
	return nil
}
