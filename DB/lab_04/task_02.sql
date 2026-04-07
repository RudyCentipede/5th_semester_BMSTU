CREATE OR REPLACE FUNCTION count_books_by_genre(genre_name text)
RETURNS integer AS
$$
    query = f"""
        SELECT COUNT(*) as count
        FROM tp.books b
        JOIN tp.genres g ON b.genre_id = g.id
        WHERE g.name ILIKE '{genre_name}'
    """
    res = plpy.execute(query)
    return res[0]["count"] if res else 0
$$
LANGUAGE plpython3u;


SELECT count_books_by_genre('драма') as books_in_genre;