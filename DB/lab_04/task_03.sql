CREATE OR REPLACE FUNCTION get_books_by_genre(genre_name text)
RETURNS TABLE (
    book_id integer,
    title text,
    publication_year integer
) AS
$$
    query = f"""
        SELECT b.id, b.title, b.publication_year
        FROM tp.books b
        JOIN tp.genres g ON b.genre_id = g.id
        WHERE g.name ILIKE '{genre_name}'
    """
    res = plpy.execute(query)

    for row in res:
        yield (row["id"], row["title"], row["publication_year"])
$$ LANGUAGE plpython3u;


SELECT * FROM get_books_by_genre('роман');