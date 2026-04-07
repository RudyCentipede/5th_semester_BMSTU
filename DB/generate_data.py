import csv
import random
from datetime import datetime, timedelta
from faker import Faker

fake = Faker('ru_RU')

RECORDS_COUNT = 1000
GENRES = ['Роман', 'Фантастика', 'Детектив', 'Фэнтези', 'Научная литература',
          'Биография', 'Поэзия', 'Драма', 'Приключения', 'Исторический']
COUNTRIES = ['Россия', 'США', 'Великобритания', 'Франция', 'Германия', 'Япония']


def generate_authors():
    authors = []
    for i in range(1, RECORDS_COUNT + 1):
        is_male = random.choice([True, False])
        if is_male:
            first_name = fake.first_name_male()
            last_name = fake.last_name_male()
        else:
            first_name = fake.first_name_female()
            last_name = fake.last_name_female()

        authors.append([
            i,
            first_name,
            last_name,
            fake.date_of_birth(minimum_age=25, maximum_age=90),
            random.choice(COUNTRIES),
            fake.text(max_nb_chars=200).replace('\n', ' ').replace('"', "'")
        ])
    return authors


def generate_books():
    books = []
    for i in range(1, RECORDS_COUNT + 1):
        books.append([
            i,
            fake.catch_phrase().replace('"', "'"),
            random.randint(1, len(GENRES)),
            random.randint(1800, 2023),
            fake.company().replace('"', "'"),
            random.randint(100, 1000)
        ])
    return books


def generate_readers():
    readers = []
    emails_used = set()

    fake_en = Faker('en_US')

    for i in range(1, RECORDS_COUNT + 1):
        sex = random.choice(['М', 'Ж'])
        if sex == 'М':
            first_name = fake.first_name_male()
            last_name = fake.last_name_male()
            middle_name = fake.middle_name_male()

            first_name_en = fake_en.first_name_male().lower()
            last_name_en = fake_en.last_name_male().lower()
        else:
            first_name = fake.first_name_female()
            last_name = fake.last_name_female()
            middle_name = fake.middle_name_female()

            first_name_en = fake_en.first_name_female().lower()
            last_name_en = fake_en.last_name_female().lower()

        base_email = f"{first_name_en}.{last_name_en}"
        email = f"{base_email}@example.com"
        counter = 1
        while email in emails_used:
            email = f"{base_email}{counter}@example.com"
            counter += 1
        emails_used.add(email)

        readers.append([
            i,  # id
            last_name,
            first_name,
            middle_name,
            fake.address().replace('\n', ', ').replace('"', "'"),
            sex,
            fake.date_of_birth(minimum_age=14, maximum_age=80),
            email,
            fake.phone_number(),
            fake.date_between(start_date='-5y', end_date='today')
        ])
    return readers


def generate_genres():
    genres = []
    for i, genre_name in enumerate(GENRES, 1):
        genres.append([i, genre_name])
    return genres


def generate_book_loans():
    loans = []
    for i in range(1, RECORDS_COUNT + 1):
        loan_date = fake.date_between(start_date='-2y', end_date='today')
        due_date = loan_date + timedelta(days=random.randint(7, 30))

        if random.random() < 0.7:
            return_date = loan_date + timedelta(days=random.randint(1, 29))
            status = 'Возвращена'
        else:
            return_date = ''
            if due_date < datetime.now().date():
                status = 'Просрочена'
            else:
                status = 'Выдана'

        loans.append([
            i,
            random.randint(1, RECORDS_COUNT),
            random.randint(1, RECORDS_COUNT),
            random.randint(1, RECORDS_COUNT),
            loan_date,
            due_date,
            return_date,
            status
        ])
    return loans


# def generate_book_ratings():
#     ratings = []
#     used_pairs = set()
#
#     for i in range(2000):
#         book_id = random.randint(1, RECORDS_COUNT)
#         reader_id = random.randint(1, RECORDS_COUNT)
#
#         while (book_id, reader_id) in used_pairs:
#             book_id = random.randint(1, RECORDS_COUNT)
#             reader_id = random.randint(1, RECORDS_COUNT)
#
#         used_pairs.add((book_id, reader_id))
#
#         rating = random.randint(1, 5)
#
#         if random.random() < 0.3:
#             review_text = ''
#         else:
#             review_text = fake.text(max_nb_chars=100).replace('\n', ' ').replace('"', "'")
#
#         created_at = fake.date_time_between(start_date='-2y', end_date='now')
#
#         ratings.append([
#             book_id,
#             reader_id,
#             rating,
#             review_text,
#             created_at.strftime('%Y-%m-%d %H:%M:%S')
#         ])
#
#     return ratings


def save_to_csv(filename, data, headers):
    with open(filename, 'w', newline='', encoding='utf-8') as f:
        writer = csv.writer(f)
        writer.writerow(headers)
        writer.writerows(data)
    print(f"Создан файл {filename} с {len(data)} записями")


def main():
    print("Начало генерации CSV файлов...")

    import os
    if not os.path.exists('csv_data'):
        os.makedirs('csv_data')

    save_to_csv('csv_data/genres.csv', generate_genres(), ['id', 'name'])

    save_to_csv('csv_data/authors.csv', generate_authors(),
                ['id', 'first_name', 'last_name', 'birth_date', 'country', 'biography'])

    save_to_csv('csv_data/books.csv', generate_books(),
                ['id', 'title', 'genre_id', 'publication_year', 'publisher', 'page_count'])  # Исправлено на genre_id

    save_to_csv('csv_data/readers.csv', generate_readers(),
                ['id', 'surname', 'name', 'middle_name', 'address', 'sex', 'birthday',
                 'email', 'phone', 'registration_date'])

    save_to_csv('csv_data/book_loans.csv', generate_book_loans(),
                ['id', 'book_id', 'author_id', 'reader_id', 'loan_date', 'due_date',
                 'return_date', 'status'])

    # save_to_csv('csv_data/book_ratings.csv', generate_book_ratings(),
    #             ['book_id', 'reader_id', 'rating', 'review_text', 'created_at'])


    print("Все CSV файлы успешно созданы!")


if __name__ == "__main__":
    main()