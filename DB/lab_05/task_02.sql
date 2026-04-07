CREATE TABLE tp.books_import_json (
    data jsonb
);

INSERT INTO tp.books_import_json (data)
VALUES ((pg_read_file('/tmp/books.json'))::jsonb);

CREATE TABLE tp.books_restored AS
SELECT
    (elem->>'id')::int AS id,
    elem->>'title' AS title,
    (elem->>'genre_id')::int AS genre_id,
    (elem->>'publication_year')::int AS publication_year,
    elem->>'publisher' AS publisher,
    (elem->>'page_count')::int AS page_count
FROM tp.books_import_json,
LATERAL jsonb_array_elements(data) AS elem;

ALTER TABLE tp.books_restored
    ADD CONSTRAINT pk_books_restored PRIMARY KEY(id),
    ADD CONSTRAINT fk_books_restored_genre FOREIGN KEY(genre_id) REFERENCES tp.genres(id),
    ADD CONSTRAINT chk_publication_year_restored CHECK(publication_year BETWEEN 1500 AND EXTRACT(YEAR FROM CURRENT_DATE)),
    ADD CONSTRAINT chk_page_count_restored CHECK(page_count > 0);
