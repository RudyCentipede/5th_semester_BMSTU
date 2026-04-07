CREATE OR REPLACE VIEW tp.reader_loan_view AS
SELECT
    r.id as reader_id,
    r.surname || ' ' || r.name as reader_full_name,
    r.email,
    bl.id as loan_id,
    b.title as book_title,
    bl.loan_date,
    bl.due_date,
    bl.status
FROM tp.readers r
LEFT JOIN tp.book_loans bl ON r.id = bl.reader_id
LEFT JOIN tp.books b ON bl.book_id = b.id;



CREATE OR REPLACE FUNCTION tp.instead_of_insert_reader_loan()
RETURNS TRIGGER
AS $$
DECLARE
    v_reader_id integer;
    v_book_id integer;
    v_author_id integer;
    v_loan_id integer;
    v_reader_surname text;
    v_reader_name text;
BEGIN
    RAISE NOTICE 'Попытка вставки через представление tp.reader_loan_view';
    RAISE NOTICE 'Читатель: %, Книга: %, Дата выдачи: %',
                 NEW.reader_full_name, NEW.book_title, NEW.loan_date;

    v_reader_surname := SPLIT_PART(NEW.reader_full_name, ' ', 1);
    v_reader_name := SPLIT_PART(NEW.reader_full_name, ' ', 2);

    SELECT id INTO v_reader_id
    FROM tp.readers
    WHERE surname = v_reader_surname AND name = v_reader_name
    LIMIT 1;

    SELECT id INTO v_book_id
    FROM tp.books
    WHERE title = NEW.book_title
    LIMIT 1;

    SELECT COALESCE(
        (SELECT author_id FROM tp.book_loans WHERE book_id = v_book_id LIMIT 1),
        (SELECT MIN(id) FROM tp.authors)
    ) INTO v_author_id;

    IF v_reader_id IS NULL THEN
        RAISE NOTICE 'Читатель % не найден. Используется читатель по умолчанию (ID=1)', new.reader_full_name;
        v_reader_id := 1;
    END IF;

    IF v_book_id IS NULL THEN
        RAISE NOTICE 'Книга % не найдена. Используется книга по умолчанию (ID=1)', new.book_title;
        v_book_id := 1;
    END IF;

    SELECT COALESCE(MAX(id), 0) + 1 INTO v_loan_id FROM tp.book_loans;

    INSERT INTO tp.book_loans (id, book_id, author_id, reader_id, loan_date, due_date, return_date, status)
    VALUES (
        v_loan_id,
        v_book_id,
        v_author_id,
        v_reader_id,
        COALESCE(new.loan_date, CURRENT_DATE),
        COALESCE(new.due_date, CURRENT_DATE + 14),
        NULL,
        COALESCE(new.status, 'Выдана')
    );

    RAISE NOTICE 'Успешно создан займ ID % для читателя ID % и книги ID %', v_loan_id, v_reader_id, v_book_id;

    RETURN new;
END;
$$
LANGUAGE plpgsql;


DROP TRIGGER IF EXISTS instead_of_insert_reader_loan_trigger ON tp.reader_loan_view;


CREATE TRIGGER instead_of_insert_reader_loan_trigger
    INSTEAD OF INSERT ON tp.reader_loan_view
    FOR EACH ROW
    EXECUTE FUNCTION tp.instead_of_insert_reader_loan();




INSERT INTO tp.reader_loan_view (reader_full_name, book_title, loan_date, due_date, status)
VALUES ('Иванов Иван', 'Преступление и наказание', '2024-01-20', '2024-02-03', 'Выдана');

INSERT INTO tp.reader_loan_view (reader_full_name, book_title, loan_date, due_date, status)
VALUES ('Кулагин Мечислав', 'Преступление и наказание', '2024-01-20', '2024-02-03', 'Выдана');

INSERT INTO tp.reader_loan_view (reader_full_name, book_title, loan_date, due_date, status)
VALUES ('Иванов Иван', 'Инверсный и мультимедийный инструментарий', '2024-01-20', '2024-02-03', 'Выдана');


SELECT * FROM tp.book_loans ORDER BY id DESC LIMIT 5;