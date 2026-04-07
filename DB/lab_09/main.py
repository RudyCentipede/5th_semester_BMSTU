import os
import json
import time
import csv
from random import randint, choice

import psycopg2
import redis
import matplotlib.pyplot as plt


# ----------------- CONFIG -----------------
PG_DSN = os.getenv(
    "PG_DSN",
    "dbname=postgres user=postgres password=postgres host=127.0.0.1 port=5432"
)
REDIS_HOST = os.getenv("REDIS_HOST", "localhost")
REDIS_PORT = int(os.getenv("REDIS_PORT", "5433"))  # у тебя 5433:6379
CACHE_KEY = "stats:top_books"
CACHE_TTL_SEC = int(os.getenv("CACHE_TTL_SEC", "60"))

QUERY_INTERVAL_SEC = 5
MUTATE_INTERVAL_SEC = 10


# ----------------- DB / REDIS -----------------
def connection():
    try:
        con = psycopg2.connect(PG_DSN)
        con.autocommit = True
    except Exception as e:
        print("Ошибка при подключении к Postgres:", e)
        raise
    print("Postgres подключен")
    return con


def redis_client():
    r = redis.Redis(host=REDIS_HOST, port=REDIS_PORT, db=0)
    # ping для быстрой проверки
    r.ping()
    return r


# ----------------- STAT QUERY (Library theme) -----------------
STAT_SQL = """
SELECT b.id, b.title, COUNT(*) AS loans_count
FROM tp.book_loans bl
JOIN tp.books b ON b.id = bl.book_id
GROUP BY b.id, b.title
ORDER BY loans_count DESC
LIMIT 10;
"""


def query_top_books_db(cur):
    """Возвращает список списков: [[book_id, title, loans_count], ...]"""
    cur.execute(STAT_SQL)
    rows = cur.fetchall()
    return [[int(r[0]), str(r[1]), int(r[2])] for r in rows]


def get_top_books_cached(cur, r):
    """
    Возвращает (result, hit, db_ms, redis_ms)
    Требование "мерить время самих запросов":
      - db_ms: время SQL (execute+fetchall)
      - redis_ms: время Redis GET (+json.loads при hit)
    При miss мы НЕ делаем второй DB-запрос: используем уже измеренный db_result.
    """
    # 1) мерим DB запрос (как эталон)
    t1 = time.perf_counter()
    db_result = query_top_books_db(cur)
    t2 = time.perf_counter()
    db_ms = (t2 - t1) * 1000.0

    # 2) мерим Redis GET
    t3 = time.perf_counter()
    blob = r.get(CACHE_KEY)
    if blob is not None:
        # hit: декодируем
        result = json.loads(blob.decode("utf-8"))
        t4 = time.perf_counter()
        redis_ms = (t4 - t3) * 1000.0
        return result, True, db_ms, redis_ms

    # miss: redis GET тоже измерен, но данных нет
    t4 = time.perf_counter()
    redis_ms = (t4 - t3) * 1000.0

    # заполняем кэш уже готовым db_result (не портим db_ms)
    r.setex(CACHE_KEY, CACHE_TTL_SEC, json.dumps(db_result, ensure_ascii=False).encode("utf-8"))
    return db_result, False, db_ms, redis_ms


# ----------------- MUTATIONS (every 10s) -----------------
def get_max_id(cur, table):
    cur.execute(f"SELECT COALESCE(MAX(id), 0) FROM {table};")
    return int(cur.fetchone()[0])


def pick_existing_id(cur, table):
    """
    Без ORDER BY RANDOM() (он шумит).
    Берём диапазон id и пытаемся найти существующий.
    """
    max_id = get_max_id(cur, table)
    if max_id <= 0:
        return None
    for _ in range(20):
        cand = randint(1, max_id)
        cur.execute(f"SELECT id FROM {table} WHERE id=%s;", (cand,))
        row = cur.fetchone()
        if row:
            return int(row[0])
    # fallback: последняя строка
    cur.execute(f"SELECT id FROM {table} ORDER BY id DESC LIMIT 1;")
    row = cur.fetchone()
    return int(row[0]) if row else None


def mutate_insert(cur, r):
    # создаём новую выдачу
    new_id = get_max_id(cur, "tp.book_loans") + 1

    book_id = pick_existing_id(cur, "tp.books")
    author_id = pick_existing_id(cur, "tp.authors")
    reader_id = pick_existing_id(cur, "tp.readers")
    if None in (book_id, author_id, reader_id):
        return

    # loan_date сегодня, due_date +14
    cur.execute("""
        INSERT INTO tp.book_loans(id, book_id, author_id, reader_id, loan_date, due_date, return_date, status)
        VALUES(%s,%s,%s,%s,CURRENT_DATE, CURRENT_DATE + INTERVAL '14 days', NULL, 'Выдана');
    """, (new_id, book_id, author_id, reader_id))

    # инвалидируем кэш статистики
    r.delete(CACHE_KEY)


def mutate_delete(cur, r):
    # удаляем последнюю выдачу (стабильно и без RANDOM)
    cur.execute("SELECT id FROM tp.book_loans ORDER BY id DESC LIMIT 1;")
    row = cur.fetchone()
    if not row:
        return
    loan_id = int(row[0])
    cur.execute("DELETE FROM tp.book_loans WHERE id=%s;", (loan_id,))
    r.delete(CACHE_KEY)


def mutate_update(cur, r):
    # закрываем (возвращаем) последнюю "Выдана"
    cur.execute("""
        SELECT id FROM tp.book_loans
        WHERE status='Выдана'
        ORDER BY id DESC
        LIMIT 1;
    """)
    row = cur.fetchone()
    if not row:
        return
    loan_id = int(row[0])

    cur.execute("""
        UPDATE tp.book_loans
        SET status='Возвращена', return_date=CURRENT_DATE
        WHERE id=%s;
    """, (loan_id,))
    r.delete(CACHE_KEY)


def do_mutation(cur, r, mode):
    if mode == "insert":
        mutate_insert(cur, r)
    elif mode == "delete":
        mutate_delete(cur, r)
    elif mode == "update":
        mutate_update(cur, r)
    # mode none -> no-op


# ----------------- EXPERIMENT LOOP -----------------
def run_experiment(mode: str, duration_sec: int, out_csv: str):
    """
    mode: none|insert|delete|update
    duration_sec: сколько секунд крутить эксперимент
    Каждые 5 сек: замер
    Каждые 10 сек: мутация (если mode != none)
    """
    con = connection()
    cur = con.cursor()
    r = redis_client()

    # лучше начать с чистого ключа
    r.delete(CACHE_KEY)

    rows = []
    start = time.time()
    next_query = start
    next_mutate = start

    while True:
        now = time.time()
        if now - start >= duration_sec:
            break

        # 10 секунд: мутация
        if mode != "none" and now >= next_mutate:
            do_mutation(cur, r, mode)
            next_mutate += MUTATE_INTERVAL_SEC

        # 5 секунд: замер
        if now >= next_query:
            result, hit, db_ms, redis_ms = get_top_books_cached(cur, r)
            # result можно не печатать, но оставлю при желании:
            # print("TOP:", result)

            rows.append({
                "ts": time.strftime("%Y-%m-%dT%H:%M:%S", time.localtime(now)),
                "mode": mode,
                "db_ms": round(db_ms, 3),
                "redis_ms": round(redis_ms, 3),
                "cache_hit": 1 if hit else 0
            })

            print(f"[{mode}] db={db_ms:.3f}ms redis={redis_ms:.3f}ms hit={hit}")
            next_query += QUERY_INTERVAL_SEC

        time.sleep(0.02)

    # write CSV
    with open(out_csv, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=["ts", "mode", "db_ms", "redis_ms", "cache_hit"])
        w.writeheader()
        w.writerows(rows)

    cur.close()
    con.close()
    r.close()
    print("Готово:", out_csv)
    return out_csv


# ----------------- PLOTS -----------------
def plot_lines(csv_file: str, title: str):
    xs, dbs, reds = [], [], []
    with open(csv_file, "r", encoding="utf-8") as f:
        rd = csv.DictReader(f)
        for i, row in enumerate(rd):
            xs.append(i)
            dbs.append(float(row["db_ms"]))
            reds.append(float(row["redis_ms"]))

    plt.figure()
    plt.plot(xs, dbs, marker="o", label="Postgres (SQL time)")
    plt.plot(xs, reds, marker="s", label="Redis (GET+decode time)")
    plt.title(title)
    plt.xlabel("Measurement index")
    plt.ylabel("ms")
    plt.grid(True)
    plt.legend()
    plt.show()


def plot_bars_avg(csv_file: str, title: str):
    dbs, reds = [], []
    with open(csv_file, "r", encoding="utf-8") as f:
        rd = csv.DictReader(f)
        for row in rd:
            dbs.append(float(row["db_ms"]))
            reds.append(float(row["redis_ms"]))

    avg_db = sum(dbs) / len(dbs) if dbs else 0
    avg_redis = sum(reds) / len(reds) if reds else 0

    plt.figure()
    plt.bar(["DB", "Redis"], [avg_db, avg_redis])
    plt.title(title)
    plt.ylabel("Average ms")
    plt.grid(True, axis="y")
    plt.show()


# ----------------- MAIN -----------------
if __name__ == "__main__":
    print(
        "Режимы:\n"
        " 1) none   - без изменения данных\n"
        " 2) insert - добавление строки каждые 10 секунд\n"
        " 3) delete - удаление строки каждые 10 секунд\n"
        " 4) update - изменение строки каждые 10 секунд\n"
    )

    mode = input("Введи режим (none/insert/delete/update): ").strip()
    duration = int(input("Длительность эксперимента (сек), напр. 180: ").strip())

    out = f"{mode}.csv"
    run_experiment(mode=mode, duration_sec=duration, out_csv=out)

    plot_lines(out, f"Mode: {mode} (lines)")
    plot_bars_avg(out, f"Mode: {mode} (average)")
