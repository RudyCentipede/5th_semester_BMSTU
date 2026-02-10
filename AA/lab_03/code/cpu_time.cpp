#include "cpu_time.h"

void generate_sequence_array(int *arr, size_t length) {
  for (size_t i = 0; i < length - 1; i++) {
    arr[i] = 1;
  }
  arr[length - 1] = 2;
}

static long long get_nanotime() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

void measure_cpu_time(size_t min_len, size_t max_len, size_t step) {
  FILE *csv_file = fopen("results.csv", "w");
  if (!csv_file) {
    printf("Ошибка создания файла результатов\n");
    return;
  }

  fprintf(csv_file, "Длина;Рекурсивный;Нерекурсивный\n");
  printf("+----------+-----------------------+\n");
  printf("|  Длина   |    Время (нс)         |\n");
  printf("|          |  Рекурс.  | Итерац.  |\n");
  printf("+----------+-----------+----------+\n");

  for (size_t len = min_len; len <= max_len; len += step) {
    int *sequence = new int[len + 1];
    generate_sequence_array(sequence, len);

    long long total_recursive = 0, total_iterative = 0;

    for (int iter = 0; iter < ITER_NUM; iter++) {
      long long start = get_nanotime();
      count_ones_recursive(sequence, 0);
      long long end = get_nanotime();
      total_recursive += (end - start);
    }

    for (int iter = 0; iter < ITER_NUM; iter++) {
      long long start = get_nanotime();
      count_ones_iterative(sequence);
      long long end = get_nanotime();
      total_iterative += (end - start);
    }

    delete[] sequence;

    double avg_recursive = (double)total_recursive / ITER_NUM;
    double avg_iterative = (double)total_iterative / ITER_NUM;

    printf("| %8zu | %9.2f | %8.2f |\n", len, avg_recursive, avg_iterative);
    fprintf(csv_file, "%zu;%.2f;%.2f\n", len, avg_recursive, avg_iterative);
  }

  printf("+----------+-----------+----------+\n");
  fclose(csv_file);
  printf("Результаты сохранены в results.csv\n");
}