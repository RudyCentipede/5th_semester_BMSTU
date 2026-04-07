package main

import (
	"database/sql"
	"fmt"
	"log"
	"time"

	_ "github.com/lib/pq"
)

func authors_total(db *sql.DB) {

	var authors int
	err := db.QueryRow("SELECT COUNT(*) FROM tp.authors").Scan(&authors)
	if err != nil {
		log.Fatal(err)
	}
	fmt.Printf("%d авторов всего\n", authors)
}

func books_by_author(db *sql.DB, id int) {
	query := `
        SELECT b.id, b.title, g.name as genre, 
               a.first_name, a.last_name
        FROM tp.books b
        JOIN tp.book_loans bl ON b.id = bl.book_id
        JOIN tp.authors a ON bl.author_id = a.id
        JOIN tp.genres g ON b.genre_id = g.id
        WHERE a.id = $1`

	rows, err := db.Query(query, id)
	if err != nil {
		log.Printf("Ошибка выполнения JOIN запросов: %v", err)
		return
	}
	defer rows.Close()
	fmt.Println("====================================================================================================")
	fmt.Printf("|%-5s|%-55s|%-20s|%-15s|\n", "ID", "Название", "Автор", "Жанр")
	fmt.Println("====================================================================================================")

	found := false
	for rows.Next() {
		var id int
		var title, genre, firstName, lastName string

		err := rows.Scan(&id, &title, &genre, &firstName, &lastName)
		if err != nil {
			log.Printf("Ошибка сканирования строки: %v", err)
			continue
		}

		author := firstName + " " + lastName
		fmt.Printf("|%-5d|%-55s|%-20s|%-15s|\n", id, title, author, genre)
		fmt.Println("----------------------------------------------------------------------------------------------------")
		found = true
	}

	if !found {
		fmt.Printf("Этот автор ещё не выдавал книг\n")
	}
}

func top_authors(db *sql.DB) {
	query := `
        WITH author_books AS (
            SELECT 
                a.last_name || ' ' || a.first_name as author_name,
                COUNT(DISTINCT bl.book_id) as book_count
            FROM tp.authors a
            LEFT JOIN tp.book_loans bl ON a.id = bl.author_id
            GROUP BY a.id, a.first_name, a.last_name
        )
        SELECT 
            author_name,
            book_count,
            ROW_NUMBER() OVER (ORDER BY book_count DESC, author_name) as position
        FROM author_books
        ORDER BY position
        LIMIT 10`

	rows, err := db.Query(query)
	if err != nil {
		log.Printf("Ошибка выполнения запроса: %v", err)
		return
	}
	defer rows.Close()

	fmt.Println("Место | Автор                | Книг")
	fmt.Println("------|----------------------|------")

	for rows.Next() {
		var authorName string
		var bookCount, position int

		err := rows.Scan(&authorName, &bookCount, &position)
		if err != nil {
			log.Printf("Ошибка сканирования: %v", err)
			continue
		}

		fmt.Printf("%-5d | %-20s | %d\n", position, authorName, bookCount)
	}
}

func tables_metadata(db *sql.DB) {
	query := `
        SELECT table_name, column_name, data_type 
        FROM information_schema.columns 
        WHERE table_schema = 'tp'
        ORDER BY table_name, ordinal_position`

	rows, err := db.Query(query)
	if err != nil {
		log.Printf("Ошибка: %v", err)
		return
	}
	defer rows.Close()

	currentTable := ""
	for rows.Next() {
		var tableName, columnName, dataType string
		rows.Scan(&tableName, &columnName, &dataType)

		if tableName != currentTable {
			fmt.Printf("\nТаблица: %s\n", tableName)
			fmt.Println("  -------------------|-------------")
			fmt.Println("  |  Столбец         | Тип        |")
			fmt.Println("  -------------------|-------------")
			currentTable = tableName
		}

		fmt.Printf("  %-18s | %s\n", columnName, dataType)
	}
}

func cnt_books_by_author(db *sql.DB, id int) {
	var bookCount int
	err := db.QueryRow("SELECT tp.count_books_by_author($1)", id).Scan(&bookCount)
	if err != nil {
		log.Printf("Ошибка вызова скалярной функции: %v", err)
		return
	}

	var authorName string
	err = db.QueryRow("SELECT first_name || ' ' || last_name FROM tp.authors WHERE id = $1", id).Scan(&authorName)
	if err != nil {
		authorName = "неизвестный автор"
	}

	fmt.Printf("\nАвтор: %s \n", authorName)
	fmt.Printf("Количество книг: %d\n", bookCount)
}

func active_loans(db *sql.DB) {
	rows, err := db.Query("SELECT * FROM tp.get_active_loans()")
	if err != nil {
		log.Printf("Ошибка вызова табличной функции: %v", err)
		return
	}
	defer rows.Close()

	fmt.Println("-------------------------------------------------------------------------------------------------------------------------------------------------------------")
	fmt.Println("| ID займа|      Читатель        |                               Книга                                    |  Дата выдачи |  Дата возврата | Просрочено дней |")
	fmt.Println("|---------|----------------------|------------------------------------------------------------------------|--------------|----------------|-----------------|")

	found := false
	for rows.Next() {
		var loanID int
		var readerName, bookTitle string
		var loanDate, dueDate time.Time
		var daysOverdue int

		err := rows.Scan(&loanID, &readerName, &bookTitle, &loanDate, &dueDate, &daysOverdue)
		if err != nil {
			log.Printf("Ошибка сканирования: %v", err)
			continue
		}

		loanDateStr := loanDate.Format("02.01.2006")
		dueDateStr := dueDate.Format("02.01.2006")

		fmt.Printf("| %-7d | %-20s | %-70s | %-12s | %-14s | %-15d |\n",
			loanID, readerName, bookTitle, loanDateStr, dueDateStr, daysOverdue)
		found = true
	}

	if found {
		fmt.Println("-------------------------------------------------------------------------------------------------------------------------------------------------------------")
	} else {
		fmt.Println("|                                                        Нет активных выдач                                                                                 |")
		fmt.Println("-------------------------------------------------------------------------------------------------------------------------------------------------------------")
	}
}

func add_book_loan(db *sql.DB, book_id int, reader_id int, author_id int, days int) {
	_, err := db.Exec("CALL tp.loan_book($1, $2, $3, $4)", book_id, reader_id, author_id, days)
	if err != nil {
		log.Printf("Ошибка вызова хранимой процедуры: %v", err)
		return
	}

	fmt.Printf("\nКнига успешно выдана\n")
}

func postgres_version(db *sql.DB) {
	var version string
	err := db.QueryRow("SELECT version()").Scan(&version)
	if err != nil {
		log.Printf("Ошибка: %v", err)
		return
	}

	fmt.Printf("Версия СУБД: %s\n", version)
}

func create_table_book_ratings(db *sql.DB) {
	query := `
        CREATE TABLE IF NOT EXISTS tp.book_ratings (
            id SERIAL PRIMARY KEY,
            book_id INTEGER NOT NULL REFERENCES tp.books(id),
            reader_id INTEGER NOT NULL REFERENCES tp.readers(id),
            rating INTEGER NOT NULL CHECK (rating >= 1 AND rating <= 5),
            review_text TEXT,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            
            CONSTRAINT unique_book_reader_rating UNIQUE(book_id, reader_id));`

	_, err := db.Exec(query)
	if err != nil {
		log.Printf("Ошибка создания таблицы: %v", err)
		return
	}
}

func copy_data(db *sql.DB) {
	csvPath := "/var/lib/postgresql/data/csv_data/book_ratings.csv"

	query := fmt.Sprintf(`
        COPY tp.book_ratings (book_id, reader_id, rating, review_text, created_at) 
        FROM '%s' DELIMITER ',' CSV HEADER`, csvPath)

	_, err := db.Exec(query)
	if err != nil {
		log.Printf("Ошибка загрузки данных: %v", err)
	}
}

func drop_db(db *sql.DB) {

	query := `DROP schema tp Cascade`
	_, err := db.Exec(query)
	if err != nil {
		log.Printf("Ошибка удаления базы данных: %v", err)
		return
	}
	fmt.Println("База данных успешно удалена")
}

func main() {
	connStr := "host=localhost port=5432 user=postgres password=postgres dbname=postgres sslmode=disable"
	db, err := sql.Open("postgres", connStr)
	if err != nil {
		log.Fatal("Ошибка подключения к базе данных:", err)
	}

	for {
		var choice int
		fmt.Println("\n=== Меню работы с базой данных библиотеки ===")
		fmt.Println("1. Количество авторов в базе")
		fmt.Println("2. Найти книги, выданные автором")
		fmt.Println("3. Топ 10 популярных авторов")
		fmt.Println("4. Выполнить запрос к метаданным")
		fmt.Println("5. Количество выдач автора")
		fmt.Println("6. Активные выдачи")
		fmt.Println("7. Выдать книгу")
		fmt.Println("8. Версия СУБД")
		fmt.Println("9. Создать таблицу отзывов на книги")
		fmt.Println("10. Выполнить вставку данных в таблицу отзывов")
		fmt.Println("11. Удалить базу данных")
		fmt.Println("0. Выход")
		fmt.Print("Выберите пункт меню: ")

		n, err := fmt.Scanln(&choice)
		if err != nil || n != 1 || choice < 0 || choice > 11 {
			log.Fatal("Ошибка выбора пункта меню: ", err)
		}

		if choice == 0 {
			break
		}

		switch choice {
		case 1:
			authors_total(db)
		case 2:
			var id int

			fmt.Print("Введите id автора: ")
			fmt.Scanln(&id)

			books_by_author(db, id)
		case 3:
			top_authors(db)
		case 4:
			tables_metadata(db)
		case 5:
			var id int

			fmt.Print("Введите id автора: ")
			fmt.Scanln(&id)

			cnt_books_by_author(db, id)
		case 6:
			active_loans(db)
		case 7:
			var book_id, reader_id, author_id, days int

			fmt.Print("Введите ID книги: ")
			fmt.Scan(&book_id)

			fmt.Print("Введите ID читателя: ")
			fmt.Scan(&reader_id)

			fmt.Print("Введите ID автора: ")
			fmt.Scan(&author_id)

			fmt.Print("Введите срок выдачи в днях: ")
			fmt.Scan(&days)

			add_book_loan(db, book_id, reader_id, author_id, days)
		case 8:
			postgres_version(db)
		case 9:
			create_table_book_ratings(db)
		case 10:
			copy_data(db)
		case 11:
			drop_db(db)

		}
	}
	db.Close()
}
