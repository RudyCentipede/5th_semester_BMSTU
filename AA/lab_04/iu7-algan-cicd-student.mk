# Обязательно в реализации
# - Цели НЕ менять
# - Зависимости и сценарии поменять на необходимые

# Рекомендуется проект выстраивать вокруг собственных сценариев сборки
# Рекомендуется в этом файле оставить только свои вызовы "make build",
# "cmake .", "msBuild proj1.sln ./out" и операции копирования

# Сценарий интерфейса специально не носит название "makefile" -
# Вы можете отсюда обращаться к своим сценариям

# Цели и зависимости в этом и в своих makefile и его аналогах остаются
# под ответственностью студента.

# Никто специально не будет искать и ругаться на Вас, если у Вас,
# например, перекомпилируется весь проект при изменении одного модуля
# (преступное неиспользование возможностей инкрементальной сборки).
# Но если, всё-таки, преподаватели заметят грязь - то спросят.

# Собрать на сервере И положить на проверку pdf отчёта
# Допускается использование libreoffice, latex, xelatex
# UPD 2025-2026: по настоянию лектора с этого года только latex


ready/report.pdf: report/report.tex
	mkdir -p ./ready
	cd report && \
     pdflatex ./report.tex && \
     bibtex report.aux && \
     pdflatex ./report.tex && \
     pdflatex ./report.tex


	gs -sDEVICE=pdfwrite \
         -dCompatibilityLevel=1.4 \
         -dNOPAUSE \
         -dOptimize=true \
         -dQUIET \
         -dBATCH \
         -dRemoveUnusedFonts=true \
         -dRemoveUnusedImages=true \
         -dOptimizeResources=true \
         -dDetectDuplicateImages \
         -dCompressFonts=true \
         -dEmbedAllFonts=true \
         -dSubsetFonts=true \
         -dPreserveAnnots=true \
         -dPreserveMarkedContent=true \
         -dPreserveOverprintSettings=true \
         -dPreserveHalftoneInfo=true \
         -dPreserveOPIComments=true \
         -dPreserveDeviceN=true \
         -dMaxInlineImageSize=0 \
		 -sOutputFile="./ready/report_compressed.pdf" "./report/report.pdf"

	mv ./ready/report_compressed.pdf ./ready/report.pdf
	rm ./report/report.pdf


# Реализация по желанию - удалить цели, если нет реализации
# UPD 2025-2026: так как из доступных остались только Си, Си++ и Раст,
# которые имеют популярные библиотеки для модульного тестирования,
# то модульное тестирование обязательно должно проводиться на сервере
#
# Пример содержимого:
#
# { 
#     "timestamp": "2024-07-14T19:46:32+03:00",
#     "coverage": 0.1,
#     "passed": 1,
#     "failed": 0
# }
#
# "timestamp" - дататаймштамп в формате UTC с указанием зоны dtst=$(date +"%Y-%m-%dT%H:%M:%S%:z")
# "coverage" - покрытие в процентах
# "passed" - число пройденных модульных тестов при последнем тестировании
# "failed" - число проваленных модульных тестов при последнем тестировании
#

ready/app-cli-release:
	mkdir -p ./ready
	@python3 build_cli_release.py

ready/app-cli-debug:
	mkdir -p ./ready
	@python3 build_cli_debug.py

ready/app-tui-release:
	mkdir -p ./ready
	@python3 build_tui_release.py

ready/app-tui-debug:
	mkdir -p ./ready
	@python3 build_tui_debug.py


# Сборка и запуск модульных тестов прямо на сервере
ready/stud-unit-test-report.json:
	@python3 generate_test_report.py

# Очистка
.PHONY: clean
clean:
	$(RM) -f ./unit_tests/*.exe ./unit_tests/*.txt ./unit_tests/*.exe *.d ./*.o ./*.exe
	$(RM) -f ./code/*.exe ./code/*.csv ./code/*.o ./code/*.dot ./code*.svg
	$(RM) -rf ./ready

# --

# Если хочется добавить в образ на сервере что-то,
# можно обратиться к @alexodnodvorcev прямо в комментарии к MR.

# --


