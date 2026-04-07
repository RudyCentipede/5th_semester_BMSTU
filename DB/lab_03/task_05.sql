CREATE OR REPLACE PROCEDURE tp.loan_book(
    p_book_id int,
    p_reader_id int,
    p_author_id int,
    p_loan_days int DEFAULT 14
)
AS
$$
DECLARE
    due_date date;
    loan_id int;
BEGIN
    due_date := CURRENT_DATE + p_loan_days;

    SELECT COALESCE(MAX(id), 0) + 1 INTO loan_id FROM tp.book_loans;

    INSERT INTO tp.book_loans (id, book_id, author_id, reader_id, loan_date, due_date, return_date, status)
    VALUES (loan_id, p_book_id, p_author_id, p_reader_id, CURRENT_DATE, due_date, NULL, 'Выдана');

    RAISE NOTICE 'Книга успешно выдана. ID займа: %, Дата возврата: %', loan_id, due_date;
END;
$$
LANGUAGE plpgsql;


CALL tp.loan_book(1, 1, 1);
CALL tp.loan_book(1, 1, 1, 21);
