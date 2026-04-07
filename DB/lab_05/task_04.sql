-- 1
SELECT data->0 AS first_book
FROM tp.books_import_json;

-- 2
SELECT
    elem->>'title' AS title,
    elem->>'publisher' AS publisher
FROM tp.books_import_json,
LATERAL jsonb_array_elements(data) AS elem;

-- 3
SELECT
    elem->>'title' AS title,
    elem ? 'page_count' AS has_page_count
FROM tp.books_import_json,
LATERAL jsonb_array_elements(data) AS elem;

-- 4
UPDATE tp.books_import_json
SET data = (
    SELECT jsonb_agg(jsonb_set(elem, '{rating}', '5'::jsonb))
    FROM jsonb_array_elements(data) AS elem
);

-- 5
SELECT jsonb_array_elements(data) AS book
FROM tp.books_import_json;
