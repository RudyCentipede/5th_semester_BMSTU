CREATE SCHEMA IF NOT EXISTS tp;

CREATE TABLE IF NOT EXISTS tp.genres (
    id int,
    name text
);

CREATE TABLE IF NOT EXISTS tp.authors (
    id int,
    first_name text,
    last_name text,
    birth_date date,
    country text,
    biography text
);

CREATE TABLE IF NOT EXISTS tp.books (
    id int,
    title text,
    genre_id int,
    publication_year int,
    publisher text,
    page_count int
);

CREATE TABLE IF NOT EXISTS tp.readers (
    id int,
    surname text,
    name text,
    middle_name text,
    address text,
    sex text,
    birthday date,
    email text,
    phone text,
    registration_date date
);

CREATE TABLE IF NOT EXISTS tp.book_loans (
    id int,
    book_id int,
    author_id int,
    reader_id int,
    loan_date date,
    due_date date,
    return_date date,
    status text
);

