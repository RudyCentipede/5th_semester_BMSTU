CREATE OR REPLACE PROCEDURE cleanup_returned_books()
AS
$$
    result = plpy.execute("""
        DELETE FROM tp.book_loans
        WHERE status = 'Возвращена'
        AND return_date < CURRENT_DATE - INTERVAL '1 year'
        RETURNING id, book_id
    """)

    deleted_count = len(result)
    plpy.notice(f"Удалено {deleted_count} записей о возвращенных книгах")

$$
LANGUAGE plpython3u;


CALL cleanup_returned_books();