import re
import PyPDF2
from typing import List, Tuple


def find_mixed_numbering(pdf_path: str) -> Tuple[bool, List[Tuple[str, Tuple[int, int]]]]:
    """
    Поиск смешения типов нумерации иллюстрирующих элементов в PDF-документе.

    Параметры:
    pdf_path (str): Путь к PDF-файлу

    Возвращает:
    Tuple[bool, List[Tuple[str, Tuple[int, int]]]]:
        - bool: True если найдено смешение типов нумерации
        - list: список найденных элементов с координатами (страница, строка)
    """

    patterns = [
        # Рисунки: Рисунок 1, Рис. 2, Рисунок 1.1, Рис. 2.3 и т.д.
        r'(?:Рисунок|Рис\.?)\s*(\d+(?:\.\d+)?)',
        # Таблицы: Таблица 1, Табл. 2, Таблица 1.1, Табл. 2.3 и т.д.
        r'(?:Таблица|Табл\.?)\s*(\d+(?:\.\d+)?)'
    ]

    compiled_patterns = [re.compile(pattern, re.IGNORECASE) for pattern in patterns]

    found_elements = []
    numbering_types = {'таблицы': set(), 'рисунки': set()}

    try:
        with open(pdf_path, 'rb') as file:
            pdf_reader = PyPDF2.PdfReader(file)

            for page_num in range(len(pdf_reader.pages)):
                page = pdf_reader.pages[page_num]
                text = page.extract_text()

                if text:
                    lines = text.split('\n')

                    for line_num, line in enumerate(lines, 1):
                        for pattern in compiled_patterns:
                            matches = pattern.findall(line)

                            for match in matches:
                                # Определяем тип элемента
                                if 'рис' in line.lower():
                                    element_type = 'рисунки'
                                elif 'табл' in line.lower():
                                    element_type = 'таблицы'
                                else:
                                    # Пропускаем, если не можем определить тип
                                    continue

                                # Определяем тип нумерации
                                if '.' in match:
                                    num_type = 'пораздельная'
                                else:
                                    num_type = 'сквозная'

                                numbering_types[element_type].add(num_type)

                                # Сохраняем найденный элемент
                                found_elements.append((
                                    line.strip(),
                                    (page_num + 1, line_num)
                                ))

    except Exception as e:
        print(f"Ошибка при обработке файла {pdf_path}: {e}")
        return False, []

    # Проверяем наличие смешения типов нумерации
    has_mixed_numbering = False

    # Проверяем каждый тип элементов
    for element_type, types in numbering_types.items():
        if len(types) > 1:
            has_mixed_numbering = True
            break

    # Проверяем смешение между типами элементов
    if numbering_types['таблицы'] and numbering_types['рисунки']:
        if numbering_types['таблицы'] != numbering_types['рисунки']:
            has_mixed_numbering = True

    return has_mixed_numbering, found_elements


def check_pdf_files(file_paths: List[str]) -> List[dict]:
    """
    Проверка нескольких PDF-файлов на смешение типов нумерации.

    Параметры:
    file_paths (List[str]): Список путей к PDF-файлам

    Возвращает:
    List[dict]: Список результатов проверки
    """
    results = []

    for file_path in file_paths:
        try:
            has_mixed, elements = find_mixed_numbering(file_path)

            if elements:
                first_element = elements[0]
                first_coords = f"Страница {first_element[1][0]}, строка {first_element[1][1]}"
                found_items = len(elements)
            else:
                first_coords = "Не найдено"
                found_items = 0

            results.append({
                'файл': file_path,
                'смешение_нумерации': 'Да' if has_mixed else 'Нет',
                'первое_вхождение': first_coords,
                'найденные_элементы': elements[:3]  # Первые 3 элемента для примера
            })

            # Вывод подробной информации
            print(f"\n{'=' * 60}")
            print(f"Файл: {file_path}")
            print(f"Смешение типов нумерации: {'ОБНАРУЖЕНО' if has_mixed else 'Не обнаружено'}")
            print(f"Найдено элементов: {found_items}")
            print(f"Первое вхождение: {first_coords}")

            if elements:
                print("Примеры найденных элементов:")
                for i, (text, coords) in enumerate(elements[:3], 1):
                    print(f"  {i}. '{text[:50]}...' (стр. {coords[0]}, строка {coords[1]})")

        except Exception as e:
            print(f"\n{'=' * 60}")
            print(f"Ошибка при обработке файла {file_path}: {e}")
            results.append({
                'файл': file_path,
                'смешение_нумерации': 'Ошибка',
                'первое_вхождение': f"Ошибка: {str(e)[:50]}...",
                'найденные_элементы': []
            })

    return results


# Основная программа
if __name__ == "__main__":
    # Список PDF-файлов для проверки (включая предоставленные)
    pdf_files = [
        "./files/_07.pdf",
        "./files/_08.pdf",
        "./files/_00.pdf",
        "./files/_65-1.pdf",
        "./files/_10.pdf",
        "./files/_05.pdf",
        "./files/_06.pdf",
        "./files/_09.pdf",
        "./files/main (2).pdf",
        "./files/ВКР Селез.pdf"
    ]

    print("=" * 80)
    print("ПРОВЕРКА СМЕШЕНИЯ ТИПОВ НУМЕРАЦИИ В PDF-ДОКУМЕНТАХ")
    print("=" * 80)

    # Проверка файлов
    results = check_pdf_files(pdf_files)

    # Вывод результатов в табличном виде
    print("\n" + "=" * 80)
    print(f"{'Файл':<25} | {'Смешение нумерации':<20} | {'Первое вхождение':<30} | {'Найдено элементов':<15}")
    print("-" * 80)

    for result in results:
        # Извлекаем количество найденных элементов
        elements_count = len(result['найденные_элементы']) if 'найденные_элементы' in result else 0

        print(
            f"{result['файл']:<25} | {result['смешение_нумерации']:<20} | {result['первое_вхождение']:<30} | {elements_count:<15}")

    # Дополнительный анализ
    print("\n" + "=" * 80)
    print("АНАЛИЗ РЕЗУЛЬТАТОВ:")
    print("-" * 80)

    files_with_mixed = [r for r in results if r['смешение_нумерации'] == 'Да']
    files_without_mixed = [r for r in results if r['смешение_нумерации'] == 'Нет']
    files_with_error = [r for r in results if r['смешение_нумерации'] == 'Ошибка']

    print(f"Всего проверено файлов: {len(results)}")
    print(f"Файлов со смешением нумерации: {len(files_with_mixed)}")
    print(f"Файлов без смешения нумерации: {len(files_without_mixed)}")
    print(f"Файлов с ошибками обработки: {len(files_with_error)}")

    if files_with_mixed:
        print("\nФайлы с ошибками (смешение типов нумерации):")
        for file in files_with_mixed:
            print(f"  - {file['файл']}")

    if files_with_error:
        print("\nФайлы с ошибками обработки:")
        for file in files_with_error:
            print(f"  - {file['файл']}")

    # Примеры обнаруженных проблем
    print("\n" + "=" * 80)
    print("ПРИМЕРЫ ОБНАРУЖЕННЫХ ПРОБЛЕМ:")
    print("-" * 80)

    # Анализ каждого файла с смешением
    for result in files_with_mixed:
        print(f"\nФайл: {result['файл']}")

        if result['найденные_элементы']:
            print("Обнаруженные элементы с разными типами нумерации:")

            # Группируем по типу нумерации
            elements_by_type = {'сквозная': [], 'пораздельная': []}

            for text, coords in result['найденные_элементы']:
                # Извлекаем номер из текста
                for pattern in [r'(?:Рисунок|Рис\.?|Таблица|Табл\.?)\s*(\d+(?:\.\d+)?)']:
                    match = re.search(pattern, text, re.IGNORECASE)
                    if match:
                        number = match.group(1)
                        if '.' in number:
                            num_type = 'пораздельная'
                        else:
                            num_type = 'сквозная'

                        elements_by_type[num_type].append({
                            'text': text[:60] + '...' if len(text) > 60 else text,
                            'coords': coords,
                            'number': number
                        })

            for num_type, elements in elements_by_type.items():
                if elements:
                    print(f"  {num_type.capitalize()} нумерация:")
                    for elem in elements[:2]:  # Показываем первые 2 примера каждого типа
                        print(f"    - {elem['number']}: {elem['text']} (стр. {elem['coords'][0]})")

    print("\n" + "=" * 80)
    print("ПРОВЕРКА ЗАВЕРШЕНА")
    print("=" * 80)