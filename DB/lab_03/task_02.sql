CREATE OR REPLACE FUNCTION tp.get_books_by_genre(genre_name text)
RETURNS TABLE(
    book_id int,
    title text,
    publication_year int,
    publisher text
) AS
$$
BEGIN
    RETURN QUERY
    SELECT b.id, b.title, b.publication_year, b.publisher
    FROM tp.books b
    JOIN tp.genres g ON b.genre_id = g.id
    WHERE g.name = genre_name;
END;
$$
LANGUAGE plpgsql;

-- Тестирование
SELECT * FROM tp.get_books_by_genre('Исторический');