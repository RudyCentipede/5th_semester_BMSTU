CREATE OR REPLACE FUNCTION tp.count_books_by_author(a_id integer)
RETURNS integer AS
$$
DECLARE
    book_count int;
BEGIN
    SELECT COUNT(*) INTO book_count
    FROM tp.books b
    WHERE b.id IN (SELECT book_id FROM tp.book_loans WHERE tp.book_loans.author_id = a_id);

    RETURN book_count;
END;
$$
LANGUAGE plpgsql;



SELECT tp.count_books_by_author(23) as book_count;
SELECT tp.count_books_by_author(900) as book_count