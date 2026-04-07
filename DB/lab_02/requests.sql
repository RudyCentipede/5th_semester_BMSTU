-- 1. Инструкция SELECT, использующая предикат сравнения.
SELECT DISTINCT auth.id, auth.last_name, auth.first_name
FROM tp.authors AS auth
WHERE auth.country='Франция' ORDER BY auth.last_name;

-- 2. Инструкция SELECT, использующая предикат BETWEEN
SELECT DISTINCT id, registration_date
FROM tp.readers
WHERE registration_date BETWEEN '2020-01-01' AND '2020-12-31'
ORDER BY registration_date;

-- 3. Инструкция SELECT, использующая предикат LIKE.
SELECT DISTINCT id, last_name
FROM tp.authors
WHERE last_name LIKE '%ов';

-- 4. Инструкция SELECT, использующая предикат IN с вложенным подзапросом.
SELECT author_id, book_id
FROM tp.book_loans
WHERE author_id IN (SELECT id
                    FROM tp.authors
                    WHERE country='Германия')
ORDER BY author_id, book_id;

-- 5. Инструкция SELECT, использующая предикат EXISTS с вложенным подзапросом.
SELECT r.id, r.name, r.surname
FROM tp.readers r
WHERE EXISTS(SELECT 1
             FROM tp.book_loans bl
             WHERE bl.reader_id = r.id);

-- 6. Инструкция SELECT, использующая предикат сравнения с квантором.
SELECT b.id, b.title, b.page_count, g.name
FROM tp.books b JOIN tp.genres g
ON b.id = g.id
WHERE b.page_count > ALL(
    SELECT b.page_count
    WHERE g.id = 1
    );

-- 7. Инструкция SELECT, использующая агрегатные функции в выражениях столбцов.
SELECT a.id, a.first_name, a.last_name, AVG(b.page_count) as avg_pages, COUNT(b.id) as total_books
FROM tp.authors a JOIN tp.book_loans bl ON a.id = bl.author_id
                  JOIN tp.books b ON bl.book_id = b.id
GROUP BY a.id, a.first_name, a.last_name
ORDER BY a.id;

-- 8. Инструкция SELECT, использующая скалярные подзапросы в выражениях столбцов.
SELECT id, title, publication_year,
       (SELECT g.name FROM tp.genres g WHERE genre_id = g.id) as genre,
       (SELECT COUNT(*) FROM tp.book_loans bl WHERE bl.book_id = b.id) as times_borrowed
FROM tp.books b
ORDER BY times_borrowed DESC;

 -- 9. Инструкция SELECT, использующая простое выражение CASE.
SELECT id, title,
    CASE genre_id
        WHEN 1 THEN 'Роман'
        WHEN 2 THEN 'Фантастика'
        WHEN 3 THEN 'Детектив'
        ELSE 'Другое'
    END as genre_name

FROM tp.books;

-- 10. Инструкция SELECT, использующая поисковое выражение CASE.
SELECT id, title, page_count,
    CASE
        WHEN page_count < 200 THEN 'Короткая'
        WHEN page_count BETWEEN 200 AND 500 THEN 'Средняя'
        ELSE 'Длинная'
    END as length
FROM tp.books;

-- 11. Создание новой временной локальной таблицы из результирующего набора данных инструкции SELECT.
CREATE TEMP TABLE popular_books AS
SELECT b.id, b.title, g.name as genre_name, COUNT(bl.id) as borrow_count, AVG(b.page_count) as avg_pages
FROM tp.books b JOIN tp.genres g ON b.genre_id = g.id
JOIN tp.book_loans bl ON b.id = bl.book_id
GROUP BY b.id, b.title, g.name
HAVING COUNT(bl.id) > 5;
SELECT * FROM popular_books ORDER BY borrow_count DESC;

-- 12. Инструкция SELECT, использующая вложенные коррелированные подзапросы в качестве производных таблиц в предложении FROM.
SELECT 'По количеству выдач' AS criteria, title as Победитель
FROM tp.books b
JOIN (
    SELECT book_id, COUNT(*) as total_loans
    FROM tp.book_loans
    GROUP BY book_id
    ORDER BY total_loans DESC
    LIMIT 1
) AS popular ON popular.book_id = b.id

UNION

SELECT 'По количеству страниц' AS criteria, title as "Лучшая книга"
FROM tp.books b
JOIN (
    SELECT id, page_count
    FROM tp.books
    ORDER BY page_count DESC
    LIMIT 1
) AS longest ON longest.id = b.id;

-- 13. Инструкция SELECT, использующая вложенные подзапросы с уровнем вложенности 3.
SELECT 'Самая популярная книга' AS criteria, title as Название
FROM tp.books
WHERE id = (
    SELECT book_id
    FROM tp.book_loans
    GROUP BY book_id
    HAVING COUNT(*) = (
        SELECT MAX(loan_count)
        FROM (
            SELECT COUNT(*) as loan_count
            FROM tp.book_loans
            GROUP BY book_id
        ) as book_stats
    )
);

-- 14. Инструкция SELECT, консолидирующая данные с помощью предложения GROUP BY, но без предложения HAVING.
SELECT g.name as genre_name,
    COUNT(b.id) as total_books,
    AVG(b.page_count) as avg_pages,
    MIN(b.publication_year) as oldest_book,
    MAX(b.publication_year) as newest_book
FROM tp.genres g JOIN tp.books b ON g.id = b.genre_id
GROUP BY g.id, g.name
ORDER BY total_books DESC;

-- 15. Инструкция SELECT, консолидирующая данные с помощью предложения GROUP BY и предложения HAVING.
SELECT g.name as genre_name,
    COUNT(b.id) as total_books,
    AVG(b.page_count) as avg_pages,
    MIN(b.publication_year) as oldest_book,
    MAX(b.publication_year) as newest_book
FROM tp.genres g JOIN tp.books b ON g.id = b.genre_id
GROUP BY g.id, g.name
HAVING AVG(b.page_count) > 600
ORDER BY total_books DESC;

-- 16. Однострочная инструкция INSERT, выполняющая вставку в таблицу одной строки значений.
INSERT INTO tp.genres (id, name)
VALUES (11, 'Комиксы');

-- 17. Многострочная инструкция INSERT, выполняющая вставку в таблицу результирующего набора данных вложенного подзапроса.
INSERT INTO tp.book_loans (id, book_id, author_id, reader_id, loan_date, due_date, status)
SELECT
    (SELECT COALESCE(MAX(id), 0) FROM tp.book_loans) + ROW_NUMBER() OVER (ORDER BY b.id),
    b.id,
    (SELECT id FROM tp.authors ORDER BY id LIMIT 1), -- берем первого автора
    (SELECT reader_id
     FROM tp.book_loans
     GROUP BY reader_id
     ORDER BY COUNT(*) DESC
     LIMIT 1), -- самый активный читатель
    CURRENT_DATE,
    CURRENT_DATE + INTERVAL '14 days',
    'Выдана'
FROM tp.books b
WHERE b.genre_id = (SELECT id FROM tp.genres WHERE name = 'Фантастика')
LIMIT 1;

-- 18. Простая инструкция UPDATE.
UPDATE tp.books
SET page_count = page_count + 10
WHERE genre_id = (SELECT id FROM tp.genres WHERE name = 'Поэзия');

-- 19. Инструкция UPDATE со скалярным подзапросом в предложении SET.
UPDATE tp.books
SET page_count = (
    SELECT ROUND(AVG(page_count))
    FROM tp.books b2
    WHERE b2.genre_id = tp.books.genre_id
)
WHERE page_count < (
    SELECT AVG(page_count)
    FROM tp.books b3
    WHERE b3.genre_id = tp.books.genre_id
);

-- 20. Простая инструкция DELETE.
DELETE FROM tp.readers
WHERE middle_name IS NULL;

-- 21. Инструкция DELETE с вложенным коррелированным подзапросом в предложении WHERE.
DELETE FROM tp.authors
WHERE id IN (
    SELECT DISTINCT author_id
    FROM tp.book_loans
    WHERE author_id IS NULL
);

-- 22. Инструкция SELECT, использующая простое обобщенное табличное выражение
WITH author_stats (author_id, book_count, avg_pages) AS (
    SELECT
        a.id,
        COUNT(DISTINCT bl.book_id) AS total_books,
        AVG(b.page_count) AS avg_pages
    FROM tp.authors a
    JOIN tp.book_loans bl ON a.id = bl.author_id
    JOIN tp.books b ON bl.book_id = b.id
    GROUP BY a.id
)
SELECT
    AVG(book_count) AS "Среднее количество книг на автора",
    AVG(avg_pages) AS "Среднее количество страниц"
FROM author_stats;

-- 23. Инструкция SELECT, использующая рекурсивное обобщенное табличное выражение.
WITH RECURSIVE reading_history AS (
    SELECT
        bl.reader_id,
        bl.book_id,
        bl.loan_date,
        1 AS books_read,
        ARRAY[bl.book_id] AS books_history
    FROM tp.book_loans bl
    WHERE bl.loan_date = (
        SELECT MIN(loan_date)
        FROM tp.book_loans
        WHERE reader_id = bl.reader_id
    )

    UNION ALL

    SELECT
        bl.reader_id,
        bl.book_id,
        bl.loan_date,
        rh.books_read + 1,
        rh.books_history || bl.book_id
    FROM tp.book_loans bl
    JOIN reading_history rh ON bl.reader_id = rh.reader_id
    WHERE bl.loan_date > rh.loan_date
      AND bl.id = (
        SELECT id
        FROM tp.book_loans
        WHERE reader_id = bl.reader_id
          AND loan_date > rh.loan_date
        ORDER BY loan_date
        LIMIT 1
      )
)
SELECT
    rh.reader_id,
    r.surname || ' ' || r.name AS reader_name,
    rh.books_read,
    b.title AS current_book,
    rh.loan_date,
    books_history
FROM reading_history rh
JOIN tp.readers r ON rh.reader_id = r.id
JOIN tp.books b ON rh.book_id = b.id
ORDER BY rh.reader_id, rh.books_read;

-- 24. Оконные функции. Использование конструкций MIN/MAX/AVG OVER()
SELECT
    g.name AS genre_name,
    AVG(b.page_count) OVER(PARTITION BY b.genre_id) AS avg_pages_in_genre,
    MIN(b.page_count) OVER(PARTITION BY b.genre_id) AS min_pages_in_genre,
    MAX(b.page_count) OVER(PARTITION BY b.genre_id) AS max_pages_in_genre,
    AVG(b.publication_year) OVER(PARTITION BY b.genre_id) AS avg_publication_year
FROM tp.books b
JOIN tp.genres g ON b.genre_id = g.id
ORDER BY g.name, b.page_count DESC;

-- 25. Оконные фукции для устранения дублей
WITH genre_stats AS (
    SELECT
        g.id AS genre_id,
        g.name AS genre_name,
        AVG(b.page_count) AS avg_pages_in_genre,
        MIN(b.page_count) AS min_pages_in_genre,
        MAX(b.page_count) AS max_pages_in_genre,
        AVG(b.publication_year) AS avg_publication_year,
        ROW_NUMBER() OVER (PARTITION BY g.id ORDER BY g.id) AS rn
    FROM tp.books b
    JOIN tp.genres g ON b.genre_id = g.id
    GROUP BY g.id, g.name
)
SELECT
    genre_name,
    avg_pages_in_genre,
    min_pages_in_genre,
    max_pages_in_genre,
    avg_publication_year
FROM genre_stats
WHERE rn = 1
ORDER BY genre_name;


----------------------------
WITH genre_author_stats AS (
    SELECT
        g.name AS genre_name,
        a.first_name || ' ' || a.last_name AS author_name,
        COUNT(bl.id) AS loan_count,
        ROW_NUMBER() OVER (PARTITION BY g.id ORDER BY COUNT(bl.id) DESC) AS rank
    FROM tp.genres g
    JOIN tp.books b ON g.id = b.genre_id
    JOIN tp.book_loans bl ON b.id = bl.book_id
    JOIN tp.authors a ON bl.author_id = a.id
    GROUP BY g.id, g.name, a.id, a.first_name, a.last_name
)
SELECT
    genre_name AS "Жанр",
    author_name AS "Автор",
    loan_count AS "Количество выдач"
FROM genre_author_stats
WHERE rank = 1;