#!/bin/bash

check_pdf_fonts() {
    local pdf_file="$1"
    if [ ! -f "$pdf_file" ]; then
        echo "❌ Файл не найден: $pdf_file"
        return 1
    fi

    pdffonts "$pdf_file" | awk '
        NR < 3 { next }
        {
          emb = $(NF-4)
          type = $2 " " $3
          name = $1
          for(i = 2; i <= NF-5; i++)
            name = name " " $i

          if(type == "Type 3") {
            print "❌ ОБНАРУЖЕН Type 3 шрифт: " name " (type=" type ")"
            e = 1
          } else if(emb != "yes") {
            print "❌ НЕ встроен: " name " (emb=" emb ")"
            e = 1
          } else {
            print "✅ OK: " name
          }
        }
        END { exit e }'

    local result=$?
    if [ $result -eq 0 ]; then
        echo "✅ Все шрифты в порядке для $pdf_file"
    else
        echo "❌ Обнаружены проблемы со шрифтами в $pdf_file"
    fi
    return $result
}

check_pdf_fonts $1