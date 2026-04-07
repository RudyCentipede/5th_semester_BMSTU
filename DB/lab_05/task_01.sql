SELECT json_agg(row_to_json(b))
FROM tp.books b;
COPY (
    SELECT json_agg(row_to_json(b))
    FROM tp.books b
) TO '/tmp/books.json';


SELECT json_agg(row_to_json(a))
FROM tp.authors a;
COPY (
    SELECT json_agg(row_to_json(a))
    FROM tp.authors a
) TO '/tmp/authors.json';


SELECT json_agg(row_to_json(bl))
FROM tp.book_loans bl;
COPY (
    SELECT json_agg(row_to_json(bl))
    FROM tp.book_loans bl
) TO '/tmp/book_loans.json';


SELECT json_agg(row_to_json(g))
FROM tp.genres g;
COPY (
    SELECT json_agg(row_to_json(g))
    FROM tp.genres g
) TO '/tmp/genres.json';


SELECT json_agg(row_to_json(r))
FROM tp.readers r;
COPY (
    SELECT json_agg(row_to_json(r))
    FROM tp.readers r
) TO '/tmp/readers.json';

