CREATE OR REPLACE PROCEDURE tp.readers_loan_report()
AS
$$
DECLARE
    reader_record RECORD;
    reader_cursor CURSOR FOR
        SELECT r.id, r.surname, r.name, COUNT(bl.id) as loan_count
        FROM tp.readers r
        LEFT JOIN tp.book_loans bl ON r.id = bl.reader_id
        GROUP BY r.id, r.surname, r.name
        ORDER BY loan_count DESC;
BEGIN
    RAISE NOTICE '======== ОТЧЁТ ПО ЧИТАТЕЛЯМ И ИХ ЗАЙМАМ ========';

    OPEN reader_cursor;
    LOOP
        FETCH reader_cursor INTO reader_record;
        EXIT WHEN NOT FOUND;

        RAISE NOTICE 'Читатель: % %, Количество займов: %',
                     reader_record.surname,
                     reader_record.name,
                     reader_record.loan_count;
    END LOOP;

    CLOSE reader_cursor;

    RAISE NOTICE '==================================================';
END;
$$
LANGUAGE plpgsql;


CALL tp.readers_loan_report();