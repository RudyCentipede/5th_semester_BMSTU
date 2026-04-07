import json
import time
import random
import uuid
from datetime import datetime, date, timedelta
from pathlib import Path
from faker import Faker

fake = Faker("ru_RU")

OUT_DIR = Path(r"C:\nifi_in")
STATE_FILE = Path(__file__).with_name("state.json")
EVERY_SECONDS = 3

GENRES = [
    "Роман", "Фантастика", "Детектив", "Фэнтези", "Научная литература",
    "Биография", "Поэзия", "Драма", "Приключения", "Исторический"
]

BATCH = {
    "authors": 3,
    "readers": 3,
    "books": 3,
    "book_loans": 5,
}

TABLES_ORDER = ["authors", "readers", "books", "book_loans"]


def load_state():
    if STATE_FILE.exists():
        return json.loads(STATE_FILE.read_text(encoding="utf-8"))
    return {
        "seeded_genres": False,
        "counters": {"authors": 0, "readers": 0, "books": 0, "book_loans": 0},
        "ids": {"authors": [], "readers": [], "books": []},
        "used_emails": [],
        "round_robin": 0,
    }


def save_state(state):
    STATE_FILE.write_text(json.dumps(state, ensure_ascii=False, indent=2), encoding="utf-8")


def write_file(table, rows):
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    file_id = str(uuid.uuid4())
    ts = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = f"{file_id}__{table}__{ts}.json"
    path = OUT_DIR / filename
    path.write_text(json.dumps(rows, ensure_ascii=False), encoding="utf-8")
    print(f"Wrote {path} ({len(rows)} rows)")


def next_id(state, table: str) -> int:
    state["counters"][table] += 1
    new_id = state["counters"][table]
    if table in state["ids"]:
        state["ids"][table].append(new_id)
    return new_id


def pick_existing(state, table: str) -> int:
    return random.choice(state["ids"][table])


def make_genres_seed():
    return [{"id": i, "name": name} for i, name in enumerate(GENRES, 1)]


def make_author(state):
    bday = fake.date_of_birth(minimum_age=25, maximum_age=90)
    return {
        "id": next_id(state, "authors"),
        "first_name": fake.first_name(),
        "last_name": fake.last_name(),
        "birth_date": bday.isoformat(),
        "country": random.choice(["Россия", "США", "Великобритания", "Франция", "Германия", "Япония"]),
        "biography": fake.text(max_nb_chars=160),
    }


def make_reader(state):
    used_emails = set(state["used_emails"])
    sex = random.choice(["М", "Ж"])
    bday = fake.date_of_birth(minimum_age=14, maximum_age=80)
    reg_date = fake.date_between(start_date="-5y", end_date="today")

    fake_en = Faker("en_US")
    fn = fake_en.first_name().lower()
    ln = fake_en.last_name().lower()
    base = f"{fn}.{ln}"
    email = f"{base}@example.com"
    k = 1
    while email in used_emails:
        email = f"{base}{k}@example.com"
        k += 1

    used_emails.add(email)
    state["used_emails"] = list(used_emails)

    return {
        "id": next_id(state, "readers"),
        "surname": fake.last_name(),
        "name": fake.first_name(),
        "middle_name": fake.middle_name(),
        "address": fake.address().replace("\n", ", "),
        "sex": sex,
        "birthday": bday.isoformat(),
        "email": email,
        "phone": fake.phone_number(),
        "registration_date": reg_date.isoformat(),
    }


def make_book(state):
    genre_id = random.randint(1, len(GENRES))
    year = random.randint(1800, date.today().year)
    pages = random.randint(100, 1000)
    return {
        "id": next_id(state, "books"),
        "title": fake.catch_phrase().replace('"', "'"),
        "genre_id": genre_id,
        "publication_year": year,
        "publisher": fake.company().replace('"', "'"),
        "page_count": pages,
    }


def make_loan(state):
    book_id = pick_existing(state, "books")
    author_id = pick_existing(state, "authors")
    reader_id = pick_existing(state, "readers")

    loan_date = fake.date_between(start_date="-120d", end_date="today")
    due_date = loan_date + timedelta(days=random.randint(7, 30))

    if random.random() < 0.7:
        return_date = loan_date + timedelta(days=random.randint(1, max(1, (due_date - loan_date).days)))
        status = "Возвращена"
        return_date_val = return_date.isoformat()
    else:
        return_date_val = ""  # пусто -> NiFi/SQL превратит в NULL
        status = "Просрочена" if due_date < date.today() else "Выдана"

    return {
        "id": next_id(state, "book_loans"),
        "book_id": book_id,
        "author_id": author_id,
        "reader_id": reader_id,
        "loan_date": loan_date.isoformat(),
        "due_date": due_date.isoformat(),
        "return_date": return_date_val,
        "status": status,
    }


def ensure_prereqs(state):
    if len(state["ids"]["authors"]) < 3:
        return "authors"
    if len(state["ids"]["readers"]) < 3:
        return "readers"
    if len(state["ids"]["books"]) < 3:
        return "books"
    return None


def main():
    state = load_state()

    if not state["seeded_genres"]:
        write_file("genres", make_genres_seed())
        state["seeded_genres"] = True
        save_state(state)
        time.sleep(EVERY_SECONDS)

    while True:
        forced = ensure_prereqs(state)
        if forced:
            table = forced
        else:
            table = TABLES_ORDER[state["round_robin"] % len(TABLES_ORDER)]
            state["round_robin"] += 1

        n = BATCH.get(table, 3)

        if table == "authors":
            rows = [make_author(state) for _ in range(n)]
        elif table == "readers":
            rows = [make_reader(state) for _ in range(n)]
        elif table == "books":
            rows = [make_book(state) for _ in range(n)]
        elif table == "book_loans":
            rows = [make_loan(state) for _ in range(n)]
        else:
            raise ValueError("unknown table")

        write_file(table, rows)
        save_state(state)
        time.sleep(EVERY_SECONDS)


if __name__ == "__main__":
    main()
