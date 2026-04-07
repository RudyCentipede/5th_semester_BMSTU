CREATE OR REPLACE FUNCTION tp.get_available_books(
    period_start date,
    period_end date
)
RETURNS TABLE(
    book_id int,
    title text,
    author text,
    current_status text,
    available_from date
) AS $$
BEGIN
    RETURN QUERY
    SELECT
        b.id as book_id,
        b.title,
        a.first_name || ' ' || a.last_name as author,
        CASE
            WHEN NOT EXISTS (
                SELECT 1 FROM tp.book_loans bl
                WHERE bl.book_id = b.id
                AND bl.status IN ('Выдана', 'Просрочена')
                AND (bl.return_date IS NULL OR bl.return_date > period_start)
            ) THEN 'Доступна сейчас'

            WHEN EXISTS (
                SELECT 1 FROM tp.book_loans bl
                WHERE bl.book_id = b.id
                AND bl.return_date BETWEEN period_start AND period_end
            ) THEN 'Будет доступна'
            ELSE 'Недоступна'
        END as current_status,

        COALESCE(
            (SELECT MIN(bl.return_date)
             FROM tp.book_loans bl
             WHERE bl.book_id = b.id
             AND bl.return_date BETWEEN period_start AND period_end),
            period_start
        ) as available_from
    FROM tp.books b
    JOIN tp.authors a ON EXISTS (
        SELECT 1 FROM tp.book_loans bl
        WHERE bl.book_id = b.id AND bl.author_id = a.id
    )
    WHERE EXISTS (
        SELECT 1 FROM tp.book_loans bl
        WHERE bl.book_id = b.id
        AND (
            bl.return_date BETWEEN period_start AND period_end
            OR bl.loan_date > period_end
            OR NOT EXISTS (
                SELECT 1 FROM tp.book_loans bl2
                WHERE bl2.book_id = b.id
                AND bl2.status IN ('Выдана', 'Просрочена')
            )
        )
    )
    ORDER BY title;
END;
$$
LANGUAGE plpgsql;

SELECT * FROM tp.get_available_books('2024-03-01', '2024-12-31')