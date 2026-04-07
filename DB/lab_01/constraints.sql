ALTER TABLE tp.authors ADD CONSTRAINT pk_author_id PRIMARY KEY(id);
ALTER TABLE tp.books ADD CONSTRAINT pk_book_id PRIMARY KEY(id);
ALTER TABLE tp.readers ADD CONSTRAINT pk_reader_id PRIMARY KEY(id);
ALTER TABLE tp.genres ADD CONSTRAINT pk_genre_id PRIMARY KEY(id);
ALTER TABLE tp.book_loans ADD CONSTRAINT pk_loan_id PRIMARY KEY(id);


ALTER TABLE tp.books
    ADD CONSTRAINT fk_books_genre FOREIGN KEY(genre_id) REFERENCES tp.genres(id);


ALTER TABLE tp.authors
    ALTER COLUMN first_name SET NOT NULL,
    ALTER COLUMN last_name SET NOT NULL;

ALTER TABLE tp.books
    ALTER COLUMN title SET NOT NULL,
    ALTER COLUMN publication_year SET NOT NULL,
    ALTER COLUMN publisher SET NOT NULL,
    ALTER COLUMN page_count SET NOT NULL;

ALTER TABLE tp.readers
    ALTER COLUMN surname SET NOT NULL,
    ALTER COLUMN name SET NOT NULL,
    ALTER COLUMN registration_date SET NOT NULL;

ALTER TABLE tp.genres
    ALTER COLUMN name SET NOT NULL;

ALTER TABLE tp.book_loans
    ALTER COLUMN loan_date SET NOT NULL,
    ALTER COLUMN due_date SET NOT NULL,
    ALTER COLUMN status SET NOT NULL;


ALTER TABLE tp.book_loans
    ADD CONSTRAINT fk_loans_book FOREIGN KEY(book_id) REFERENCES tp.books(id),
    ADD CONSTRAINT fk_loans_author FOREIGN KEY(author_id) REFERENCES tp.authors(id),
    ADD CONSTRAINT fk_loans_reader FOREIGN KEY(reader_id) REFERENCES tp.readers(id);


ALTER TABLE tp.books
    ADD CONSTRAINT chk_publication_year CHECK(publication_year BETWEEN 1500 AND EXTRACT(YEAR FROM CURRENT_DATE)),
    ADD CONSTRAINT chk_page_count CHECK(page_count > 0);

ALTER TABLE tp.readers
    ADD CONSTRAINT chk_registration_date CHECK(registration_date <= CURRENT_DATE),
    ADD CONSTRAINT chk_birthday CHECK(birthday <= CURRENT_DATE),
    ADD CONSTRAINT chk_email_format CHECK(email ~* '^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$'),
    ADD CONSTRAINT chk_sex CHECK(sex IN ('М', 'Ж')),
    ADD CONSTRAINT UC_reader_email UNIQUE (email);

ALTER TABLE tp.book_loans
    ADD CONSTRAINT chk_loan_dates CHECK(due_date >= loan_date),
    ADD CONSTRAINT chk_return_date CHECK(return_date IS NULL OR return_date >= loan_date),
    ADD CONSTRAINT chk_status CHECK(status IN ('Выдана', 'Возвращена', 'Просрочена'));
