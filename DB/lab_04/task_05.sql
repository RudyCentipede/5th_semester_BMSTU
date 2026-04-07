CREATE OR REPLACE FUNCTION notify_new_book()
RETURNS TRIGGER AS
$$
    plpy.notice('Новая книга добавлена в таблицу tp.books')
    plpy.notice(f'ID книги = {TD["new"]["id"]}, Название = {TD["new"]["title"]}, Год издания = {TD["new"]["publication_year"]}')
$$
LANGUAGE plpython3u;


DROP TRIGGER IF EXISTS notify_new_book_trigger_clr ON tp.books;

CREATE TRIGGER notify_new_book_trigger_clr
AFTER INSERT ON tp.books
FOR EACH ROW
EXECUTE FUNCTION notify_new_book();


INSERT INTO tp.books (id, title, genre_id, publication_year, publisher, page_count)
VALUES (
    (SELECT COALESCE(MAX(id), 0) + 1 FROM tp.books),
    'Новая книга по Python',
    1,
    2025,
    'ООО Анаконда',
    3000
);


SELECT * FROM tp.books ORDER BY id DESC LIMIT 1;