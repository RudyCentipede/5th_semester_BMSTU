#!/usr/bin/env bash
set -euo pipefail

need() { command -v "$1" >/dev/null 2>&1 || {
	echo "Missing tool: $1" >&2
	exit 2
}; }
need pdffonts
if ! command -v pdfinfo >/dev/null 2>&1; then
	echo "Warning: pdfinfo not found; page-level mapping will be skipped." >&2
fi

inputs=("$@")
if [ ${#inputs[@]} -eq 0 ]; then inputs=("./report/report.pdf"); fi

# expand dirs to files
files=()
for p in "${inputs[@]}"; do
	if [ -d "$p" ]; then
		while IFS= read -r -d '' f; do files+=("$f"); done < <(find "$p" -type f -iname "*.pdf" -print0 | sort -z)
	else
		files+=("$p")
	fi
done

any_t3=false

for pdf in "${files[@]}"; do
	[ -f "$pdf" ] || {
		echo "Skip: $pdf (not found)"
		continue
	}

	tmp="$(mktemp)"
	pdffonts "$pdf" | tee "$tmp"
	echo "$pdf"

	t3_found=0
	# отметки OK/Type3 и сбор имён Type3
	mapfile -t t3_names < <(awk 'NR<=2{next} NF<2{next} ($2=="Type" && $3=="3"){print $1}' "$tmp")

	awk 'NR<=2 { next } NF<2 { next }
    {
      name=$1; type=$2 " " $3; enc=$4;
      if ($2=="Type" && $3=="3") {
        print "❌ ОБНАРУЖЕН Type 3 шрифт: " name " " type " " enc " (type=Type 3)";
        t3=1
      } else {
        print "✅ OK: " name " " type " " enc
      }
    }
    END { exit t3 }' "$tmp" || t3_found=1

	# Если есть Type3 и доступен pdfinfo — вывесить страницы с точным попаданием
	if ((t3_found)) && command -v pdfinfo >/dev/null 2>&1; then
		pages=$(pdfinfo "$pdf" 2>/dev/null | awk '/^Pages:/ {print $2}')
		if [[ -n ${pages:-} && $pages =~ ^[0-9]+$ ]]; then
			declare -A t3_by_page=()
			for ((p = 1; p <= pages; p++)); do
				pt="$(mktemp)"
				pdffonts -f "$p" -l "$p" "$pdf" >"$pt" 2>/dev/null || true
				# все Type3 на странице
				mapfile -t page_t3 < <(awk 'NR<=2{next} NF<2{next} ($2=="Type" && $3=="3"){print $1}' "$pt")
				if ((${#page_t3[@]})); then
					IFS=',' read -r -a fset <<<"$(printf "%s," "${page_t3[@]}")"
					# уникальные имена
					declare -A seen=()
					uniq=()
					for n in "${page_t3[@]}"; do
						[[ ${seen[$n]+x} ]] || {
							seen[$n]=1
							uniq+=("$n")
						}
					done
					t3_by_page[$p]="${uniq[*]}"
				fi
				rm -f "$pt"
			done
			if ((${#t3_by_page[@]})); then
				echo "Подробности (Type 3 по страницам):"
				for p in $(printf "%s\n" "${!t3_by_page[@]}" | sort -n); do
					echo "  страница $p: ${t3_by_page[$p]}"
				done
			fi
			unset t3_by_page
		fi
	fi

	rm -f "$tmp"
	if ((t3_found)); then any_t3=true; fi
done

$any_t3 && exit 1 || exit 0
