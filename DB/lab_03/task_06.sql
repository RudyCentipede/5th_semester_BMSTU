CREATE OR REPLACE PROCEDURE tp.author_connections(
    p_start_author_id int,
    INOUT connection_tree text DEFAULT '',
    INOUT level int DEFAULT 0,
    INOUT visited_authors int[] DEFAULT '{}'
)
AS $$
DECLARE
    current_author record;
    connected_authors record;
BEGIN
    SELECT first_name, last_name INTO current_author
    FROM tp.authors
    WHERE id = p_start_author_id;

    visited_authors := visited_authors || p_start_author_id;

    IF level = 0 THEN
        connection_tree := 'Автор: ' || current_author.first_name || ' ' || current_author.last_name;
    ELSE
        connection_tree := connection_tree || E'\n' || 'Связан с: ' ||
                         current_author.first_name || ' ' || current_author.last_name;
    END IF;

    FOR connected_authors IN
        SELECT DISTINCT a.id, a.first_name, a.last_name,
               (SELECT COUNT(*)
                FROM tp.book_loans bl1
                WHERE bl1.author_id = p_start_author_id
                AND bl1.book_id IN (
                    SELECT book_id FROM tp.book_loans WHERE author_id = a.id
                )) as common_books
        FROM tp.authors a
        JOIN tp.book_loans bl ON a.id = bl.author_id
        WHERE bl.book_id IN (
            SELECT book_id FROM tp.book_loans WHERE author_id = p_start_author_id
        )
        AND a.id != p_start_author_id
        AND a.id NOT IN (SELECT unnest(visited_authors))
        ORDER BY common_books DESC
        LIMIT 2
    LOOP

        level := level + 1;
        CALL tp.author_connections(
            connected_authors.id,
            connection_tree,
            level,
            visited_authors
        );
        level := level - 1;
    END LOOP;
END;
$$ LANGUAGE plpgsql;



DO
$$
DECLARE
    result_tree text := '';
    level_counter int := 0;
    visited int[] := '{}';
BEGIN
    CALL tp.author_connections(1, result_tree, level_counter, visited);
    RAISE NOTICE '%', result_tree;
END
$$;