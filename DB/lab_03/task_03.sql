CREATE OR REPLACE FUNCTION tp.get_active_loans()
RETURNS TABLE(
    loan_id int,
    reader_name text,
    book_title text,
    loan_date date,
    due_date date,
    days_overdue int
) AS
$$
BEGIN
    RETURN QUERY
    SELECT
        bl.id as loan_id,
        r.surname || ' ' || r.name as reader_name,
        b.title as book_title,
        bl.loan_date,
        bl.due_date,
        CASE
            WHEN bl.return_date IS NULL AND bl.due_date < CURRENT_DATE
            THEN CURRENT_DATE - bl.due_date
            ELSE 0
        END as days_overdue
    FROM tp.book_loans bl
    JOIN tp.readers r ON bl.reader_id = r.id
    JOIN tp.books b ON bl.book_id = b.id
    WHERE bl.status = 'Выдана' AND bl.return_date IS NULL;


    RAISE NOTICE 'Найдено активных займов: %', (
    SELECT COUNT(*)
    FROM tp.book_loans
    WHERE status = 'Выдана' AND return_date IS NULL);
END;
$$
LANGUAGE plpgsql;


SELECT * FROM tp.get_active_loans();