COPY tp.genres FROM '/var/lib/postgresql/data/csv_data/genres.csv' DELIMITER ',' CSV HEADER;
COPY tp.books FROM '/var/lib/postgresql/data/csv_data/books.csv' DELIMITER ',' CSV HEADER;
COPY tp.authors FROM '/var/lib/postgresql/data/csv_data/authors.csv' DELIMITER ',' CSV HEADER;
COPY tp.readers FROM '/var/lib/postgresql/data/csv_data/readers.csv' DELIMITER ',' CSV HEADER;
COPY tp.book_loans FROM '/var/lib/postgresql/data/csv_data/book_loans.csv' DELIMITER ',' CSV HEADER;