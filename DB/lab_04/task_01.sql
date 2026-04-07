CREATE OR REPLACE FUNCTION book_difficulty(page_cnt int)
RETURNS text AS
$$
    if page_cnt < 700:
        return 'Короткая'
    elif page_cnt < 900:
        return 'Средняя'

    return 'Длинная'
$$
LANGUAGE plpython3u;

SELECT
    id,
    title,
    page_count,
    book_difficulty(page_count) as difficulty

FROM tp.books
