CREATE OR REPLACE PROCEDURE tp.get_schema_metadata()
AS
$$
DECLARE
    table_record RECORD;
    column_record RECORD;
BEGIN
    FOR table_record IN
        SELECT table_name, table_type
        FROM information_schema.tables
        WHERE table_schema = 'tp'
        ORDER BY table_name
    LOOP
        RAISE NOTICE '================================================================';
        RAISE NOTICE 'Таблица: % (тип: %)', table_record.table_name, table_record.table_type;
        RAISE NOTICE '================================================================';

        FOR column_record IN
            SELECT column_name, data_type, is_nullable
            FROM information_schema.columns
            WHERE table_schema = 'tp' AND table_name = table_record.table_name
            ORDER BY ordinal_position
        LOOP
            RAISE NOTICE '  Колонка: % (тип: %, nullable: %)',
                         column_record.column_name,
                         column_record.data_type,
                         column_record.is_nullable;
        END LOOP;
    END LOOP;

END;
$$
LANGUAGE plpgsql;


CALL tp.get_schema_metadata();