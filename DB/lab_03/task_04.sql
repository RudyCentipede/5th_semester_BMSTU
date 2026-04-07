CREATE OR REPLACE FUNCTION tp.get_reader_recursive()
RETURNS TABLE(
    reader_id int,
    reader_name text,
    books_read int,
    current_book_title text,
    loan_date date,
    reading_sequence int[]
) AS
$$
BEGIN
    RETURN QUERY
    WITH RECURSIVE reading_journey AS (
        SELECT
            bl.reader_id,
            bl.book_id,
            bl.loan_date,
            1 AS books_count,
            ARRAY[bl.book_id] AS book_sequence
        FROM tp.book_loans bl
        WHERE bl.loan_date = (
            SELECT MIN(bl2.loan_date)
            FROM tp.book_loans bl2
            WHERE bl2.reader_id = bl.reader_id
        )

        UNION ALL

        SELECT
            bl.reader_id,
            bl.book_id,
            bl.loan_date,
            rj.books_count + 1,
            rj.book_sequence || bl.book_id
        FROM tp.book_loans bl
        JOIN reading_journey rj ON bl.reader_id = rj.reader_id
        WHERE bl.loan_date > rj.loan_date
          AND bl.id = (
            SELECT bl3.id
            FROM tp.book_loans bl3
            WHERE bl3.reader_id = bl.reader_id
              AND bl3.loan_date > rj.loan_date
            ORDER BY bl3.loan_date
            LIMIT 1
          )
    )
    SELECT
        rj.reader_id,
        r.surname || ' ' || r.name AS reader_name,
        rj.books_count AS books_read,
        b.title AS current_book_title,
        rj.loan_date,
        rj.book_sequence AS reading_sequence
    FROM reading_journey rj
    JOIN tp.readers r ON rj.reader_id = r.id
    JOIN tp.books b ON rj.book_id = b.id
    ORDER BY rj.reader_id, rj.books_count;
END;
$$
LANGUAGE plpgsql;


SELECT * FROM tp.get_reader_recursive();