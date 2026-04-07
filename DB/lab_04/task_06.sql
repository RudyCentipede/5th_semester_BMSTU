CREATE TYPE tp.genre_info AS (
    genre_id integer,
    genre_name text,
    books_count integer
);


CREATE OR REPLACE FUNCTION tp.create_genre_info(genre_id integer)
RETURNS tp.genre_info AS
$$
    result = plpy.execute(f"""
        SELECT g.id, g.name,
               COUNT(b.id) as books_count
        FROM tp.genres g
        LEFT JOIN tp.books b ON g.id = b.genre_id
        WHERE g.id = {genre_id}
        GROUP BY g.id, g.name
    """)

    if not result:
        return (genre_id, "Не найден", 0)

    genre = result[0]
    return (genre["id"], genre["name"], genre["books_count"])
$$
LANGUAGE plpython3u;


SELECT (tp.create_genre_info(2)).*;