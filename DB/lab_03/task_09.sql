CREATE OR REPLACE FUNCTION tp.notify_new_loan()
RETURNS TRIGGER
AS
$$
BEGIN
    RAISE NOTICE 'Новый займ книги добавлен в таблицу tp.book_loans';
    RAISE NOTICE 'ID займа = %, ID книги = %, ID читателя = %, Дата выдачи = %',
                 NEW.id, NEW.book_id, NEW.reader_id, NEW.loan_date;
    RETURN NEW;
END;
$$
LANGUAGE plpgsql;


DROP TRIGGER IF EXISTS notify_new_loan_trigger ON tp.book_loans;


CREATE TRIGGER notify_new_loan_trigger
AFTER INSERT ON tp.book_loans
FOR EACH ROW
EXECUTE FUNCTION tp.notify_new_loan();


INSERT INTO tp.book_loans (id, book_id, author_id, reader_id, loan_date, due_date, return_date, status)
VALUES (
    (SELECT COALESCE(MAX(id), 0) + 1 FROM tp.book_loans),
    1, 1, 1,
    CURRENT_DATE,
    CURRENT_DATE + 14,
    NULL,
    'Выдана'
);


SELECT * FROM tp.book_loans ORDER BY id DESC LIMIT 1;