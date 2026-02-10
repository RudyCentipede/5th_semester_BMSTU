#!/bin/bash
# Что, пацаны, копирайт?
# (c) Alexander Kostritsky 2024
# Если Вы найс пёрсон, то пользуйтесь, иначе - не пользуйтесь

# Проверка количества аргументов
if [ $# -ne 2 ]; then
    echo "Использование: $0 <pdf1> <pdf2>"
    echo "Проверяет, что размеры двух PDF файлов отличаются не более чем на 5%"
    exit 1
fi

pdf1="$1"
pdf2="$2"

# Проверка существования файлов
if [ ! -f "$pdf1" ]; then
    echo "Ошибка: Файл '$pdf1' не существует"
    exit 1
fi

if [ ! -f "$pdf2" ]; then
    echo "Ошибка: Файл '$pdf2' не существует"
    exit 1
fi

# Получение размеров файлов
size1=$(stat -c%s "$pdf1" 2>/dev/null || stat -f%z "$pdf1")
size2=$(stat -c%s "$pdf2" 2>/dev/null || stat -f%z "$pdf2")

# Проверка успешности получения размеров
if [ -z "$size1" ] || [ -z "$size2" ]; then
    echo "Ошибка: Не удалось получить размеры файлов"
    exit 1
fi

# Вычисление разницы в процентах
if [ "$size1" -eq 0 ] || [ "$size2" -eq 0 ]; then
    echo "Ошибка: Один из файлов имеет нулевой размер"
    exit 1
fi

# Вычисление разницы в процентах
if [ "$size1" -gt "$size2" ]; then
    diff_percent=$(((size1 - size2) * 100 / size1))
else
    diff_percent=$(((size2 - size1) * 100 / size2))
fi

# Вывод информации
echo "Размер $pdf1: $size1 байт"
echo "Размер $pdf2: $size2 байт"
echo "Разница: $diff_percent%"

# Проверка условия
if [ "$diff_percent" -le 5 ]; then
    echo "✅ Размеры файлов отличаются не более чем на 5%"
    exit 0
else
    echo "❌ Размеры файлов отличаются более чем на 5%"
    exit 1
fi
