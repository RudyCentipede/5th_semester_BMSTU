import re
from typing import Any, Dict, List, Tuple
from PyPDF2 import PdfReader
import argparse
from pathlib import Path

CAPTION_RE = re.compile(
    r'(?iu)\b(?P<kind>рис(?:унок)?\.?|табл(?:ица)?\.?)\s*'
    r'(?:№\s*)?'
    r'(?:[-–—:]\s*)?'
    r'(?P<num>\d+(?:\.\d+)*)'
)
CONTINUOUS_NUM_RE = re.compile(r'^\d+$')
SECTIONAL_NUM_RE  = re.compile(r'^\d+\.\d+(?:\.\d+)*$')


def check_mixed_numbering_in_pdf(pdf_path: str) -> Tuple[bool, List[Tuple[str, Tuple[int, int]]]]:
    """
    Возвращает кортеж:
      (mixed_found, matches)

    mixed_found: bool
      True  — найдено смешение стилей нумерации (ошибка по варианту)
      False — смешение не найдено

    matches: list[tuple[str, (page, line)]]
      Если mixed_found == True: список строк-доказательств и координат
      (номер страницы PDF, номер строки на странице в извлеченном тексте).
      Если mixed_found == False: пустой список.
    """
    reader = PdfReader(pdf_path)

    found: List[Dict[str, Any]] = []

    for page_idx, page in enumerate(reader.pages, start=1):
        text = (page.extract_text() or "").replace("\u00A0", " ")
        lines = text.splitlines()

        for line_idx, line in enumerate(lines, start=1):
            for m in CAPTION_RE.finditer(line):
                kind_raw = m.group("kind").lower()
                kind = "figure" if kind_raw.startswith("рис") else "table"
                num = m.group("num")

                if SECTIONAL_NUM_RE.fullmatch(num):
                    style = "sectional"
                elif CONTINUOUS_NUM_RE.fullmatch(num):
                    style = "continuous"
                else:
                    continue

                found.append({
                    "page": page_idx,
                    "line": line_idx,
                    "match": m.group(0).strip(),
                    "kind": kind,
                    "style": style,
                })

    if not found:
        return (False, [])

    fig_styles = {x["style"] for x in found if x["kind"] == "figure"}
    tab_styles = {x["style"] for x in found if x["kind"] == "table"}

    mixed = False
    # смешение стилей внутри одного типа
    if len(fig_styles) > 1 or len(tab_styles) > 1:
        mixed = True
    # рисунки и таблицы оформлены разными стилями
    if fig_styles and tab_styles and fig_styles != tab_styles:
        mixed = True

    if not mixed:
        return (False, [])

    evidence: List[Tuple[str, Tuple[int, int]]] = []

    # Если смешение между рисунками и таблицами — берём по одной подписи каждого типа
    if fig_styles and tab_styles and fig_styles != tab_styles:
        fig = next((r for r in found if r["kind"] == "figure"), None)
        tab = next((r for r in found if r["kind"] == "table"), None)
        if fig:
            evidence.append((fig["match"], (fig["page"], fig["line"])))
        if tab:
            evidence.append((tab["match"], (tab["page"], tab["line"])))
    else:
        # Иначе — минимум по одной строке на каждый стиль
        picked = set()
        for r in found:
            if r["style"] not in picked:
                evidence.append((r["match"], (r["page"], r["line"])))
                picked.add(r["style"])
            if len(picked) >= 2:
                break

    return (True, evidence)


def main() -> int:
    parser = argparse.ArgumentParser(description="Проверка смешения нумерации рисунков/таблиц в PDF.")
    parser.add_argument("path", help="Путь к PDF-файлу или папке с PDF")
    args = parser.parse_args()

    p = Path(args.path)

    pdf_files = []
    if p.is_file() and p.suffix.lower() == ".pdf":
        pdf_files = [p]
    elif p.is_dir():
        pdf_files = sorted(p.glob("*.pdf"))
    else:
        print("Ошибка: укажите PDF-файл или папку с PDF.")
        return 2

    # Заголовок таблицы
    print(f"{'pdf-файл':60} | {'успех':6} | {'координаты первого нахождения':30}")
    print("-" * 105)

    for pdf in pdf_files:
        mixed, matches = check_mixed_numbering_in_pdf(str(pdf))
        success = "да" if mixed else "нет"
        first_coord = "-"
        if matches:
            _, (page, line) = matches[0]
            first_coord = f"стр. {page}, строка {line}"
        print(f"{pdf.name:60} | {success:6} | {first_coord:30}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

