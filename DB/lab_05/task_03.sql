ALTER TABLE tp.readers
ADD COLUMN additional_info jsonb;


UPDATE tp.readers
SET additional_info = jsonb_build_object(
    'membership_level', 'Платина',
    'notifications', jsonb_build_object(
        'email_enabled', false,
        'sms_enabled', true
    )
)
WHERE id = 2;

SELECT id, surname, name, additional_info
FROM tp.readers
WHERE id = 2;
