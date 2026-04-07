CREATE OR REPLACE PROCEDURE random_book_faker()
AS
$$
from faker import Faker
import random

fake = Faker('ru_RU')

genres = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]


result = plpy.execute("SELECT COALESCE(MAX(id), 0) + 1 as next_id FROM tp.books")
book_id = result[0]["next_id"]


title = fake.catch_phrase()
genre_id = random.choice(genres)
publication_year = random.randint(1900, 2023)
publisher = fake.company()
page_count = random.randint(100, 800)

query = f"""
INSERT INTO tp.books (id, title, genre_id, publication_year, publisher, page_count)
VALUES ({book_id}, '{title.replace("'", "''")}', {genre_id}, {publication_year}, '{publisher.replace("'", "''")}', {page_count})
"""

plpy.execute(query)

$$
LANGUAGE plpython3u;


CALL random_book();