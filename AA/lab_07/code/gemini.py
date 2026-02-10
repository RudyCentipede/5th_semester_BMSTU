import re
import os
from PyPDF2 import PdfReader

# 1. Регулярные выражения
# Ищет: Начало строки, пробелы, "Таблица", пробелы, цифры (опционально с точкой и цифрами)
REGEX_TABLE = r"^\s*Таблица\s+(\d+(?:\.\d+)?)"

# Ищет: Начало строки, пробелы, "Рисунок" или "Рис.", пробелы, цифры (опционально с точкой и цифрами)
REGEX_FIGURE = r"^\s*(?:Рисунок|Рис\.)\s+(\d+(?:\.\d+)?)"


def determine_numbering_type(number_str):
    """
    Определяет тип нумерации по строковому представлению числа.
    Возвращает: 'sectional' (пораздельная) или 'continuous' (сквозная).
    """
    if '.' in number_str:
        return 'sectional'
    return 'continuous'


def check_mixed_numbering(pdf_path):
    """
    Функция проверяет pdf-файл на наличие смешения типов нумерации
    иллюстрирующих элементов (таблиц и рисунков).

    Аргументы:
        pdf_path (str): Путь к файлу PDF.

    Возвращает:
        tuple: (
            bool: Истина, если найдено смешение (ошибка), иначе Ложь.
            list: Список кортежей [('найденная строка', (номер_страницы, номер_строки)), ...]
                  Если смешения нет, список пустой.
        )
    """
    found_elements = []  # Список всех найденных элементов для анализа

    try:
        reader = PdfReader(pdf_path)
    except Exception as e:
        print(f"Ошибка при чтении файла {pdf_path}: {e}")
        return False, []

    # Проход по всем страницам документа
    for page_idx, page in enumerate(reader.pages):
        text = page.extract_text()
        if not text:
            continue

        lines = text.split('\n')
        for line_idx, line in enumerate(lines):
            # Проверка на Таблицу
            table_match = re.search(REGEX_TABLE, line, re.IGNORECASE)
            if table_match:
                num_str = table_match.group(1)
                num_type = determine_numbering_type(num_str)
                found_elements.append({
                    'type': 'table',
                    'style': num_type,
                    'text': line.strip(),
                    'coords': (page_idx + 1, line_idx + 1)  # Нумерация для людей (с 1)
                })
                continue  # Если нашли таблицу, рисунок в этой же строке маловероятен (в начале строки)

            # Проверка на Рисунок
            fig_match = re.search(REGEX_FIGURE, line, re.IGNORECASE)
            if fig_match:
                num_str = fig_match.group(1)
                num_type = determine_numbering_type(num_str)
                found_elements.append({
                    'type': 'figure',
                    'style': num_type,
                    'text': line.strip(),
                    'coords': (page_idx + 1, line_idx + 1)
                })

    # Анализ собранных данных
    table_styles = set(item['style'] for item in found_elements if item['type'] == 'table')
    fig_styles = set(item['style'] for item in found_elements if item['type'] == 'figure')

    has_error = False

    # 1. Проверка внутренней консистентности (нельзя Таблица 1 и Таблица 1.1 в одном доке)
    # Хотя вариант делает акцент на смешении МЕЖДУ типами, обычно внутренняя смешанность тоже ошибка.
    # Но строго по варианту: "нельзя таблицы сквозной, а рисунки пораздельной".

    # Проверка конфликта между Таблицами и Рисунками
    # Конфликт, если присутствуют оба типа элементов, и их стили не пересекаются или явно противоречат

    current_table_style = list(table_styles)[0] if table_styles else None
    current_fig_style = list(fig_styles)[0] if fig_styles else None

    # Логика определения ошибки согласно варианту:
    # "Нельзя таблицы нумеровать сквозной (continuous), а рисунки пораздельной (sectional)"
    # И наоборот. То есть стили должны совпадать, если присутствуют оба вида элементов.

    if current_table_style and current_fig_style:
        if current_table_style != current_fig_style:
            has_error = True
        # Также ошибка, если внутри одной категории смешанные стили (например, set имеет длину > 1)
        if len(table_styles) > 1 or len(fig_styles) > 1:
            has_error = True
    elif len(table_styles) > 1 or len(fig_styles) > 1:
        # Если есть только таблицы, но они смешанные (1 и 1.1) - это тоже смешение типов нумерации
        has_error = True

    if has_error:
        # Формируем требуемый список кортежей
        result_list = [(item['text'], item['coords']) for item in found_elements]
        return True, result_list
    else:
        return False, []


def main():
    print("Программа проверки нумерации в PDF-документах.")
    file_path = input("Введите путь к pdf-файлу: ").strip()

    # Убираем кавычки, если пользователь скопировал путь как "C:\path\..."
    if file_path.startswith('"') and file_path.endswith('"'):
        file_path = file_path[1:-1]

    if not os.path.exists(file_path):
        print("Файл не найден.")
        return

    is_mixed, details = check_mixed_numbering(file_path)

    print("-" * 50)
    print(f"Файл: {os.path.basename(file_path)}")

    if is_mixed:
        print("Результат: ОБНАРУЖЕНО смешение типов нумерации (Истина).")
        print("Найденные элементы, вызвавшие подозрение:")
        print(f"{'Стр.':<5} | {'Стр.док.':<8} | {'Текст'}")
        print("-" * 50)
        for text, coords in details:
            page_num, line_num = coords
            # Обрезаем слишком длинный текст для вывода
            display_text = (text[:60] + '..') if len(text) > 60 else text
            print(f"{page_num:<5} | {line_num:<8} | {display_text}")
    else:
        print("Результат: Смешения типов нумерации не обнаружено (Ложь).")
        # Список пустой, выводить нечего

    print("-" * 50)


if __name__ == "__main__":
    main()