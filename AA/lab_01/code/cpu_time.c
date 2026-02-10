#include "cpu_time.h"

void fill_matrix_random(matrix_t *matrix, int max_value) {
  for (size_t i = 0; i < matrix->n; i++)
    for (size_t j = 0; j < matrix->m; j++)
      matrix->data[i][j] = rand() % max_value;
}

void measure_cpu_time(size_t start_len, size_t stop_len, size_t step) {
  FILE *csv_file = fopen("../report/tables/results2.csv", "w");
  if (csv_file == NULL) {
    printf("Ошибка открытия файла *.csv для записи\n");
    return;
  }

  fprintf(csv_file,
          "Размер;Стандартный;Винограда;Винограда_оптимизированный\n");

  printf("+--------+---------------------------------------------+\n");
  printf("| %5s | %16s %s %16s |\n", "Размер", "", "Время, мс", "");
  printf("|        |------------+---------------+----------------|\n");
  printf("| %6s | %18s | %22s | %20s |\n", "", "Стандарт.", " Винограда ",
         "Винограда опт.");
  printf("|--------|------------|---------------|----------------|\n");

  for (size_t i = start_len; i <= stop_len; i += step) {
    matrix_t a = {0};
    matrix_t b = {0};
    matrix_t c = {0};

    a.n = i;
    a.m = i;
    b.n = i;
    b.m = i;
    c.n = i;
    c.m = i;

    if (alloc_matrix(&a) != 0) {
      printf("| %6zu | Ошибка выделения памяти                    |\n", i);
      printf("+--------+------------+---------------+----------------+\n");
      break;
    }

    if (alloc_matrix(&b) != 0) {
      free_matrix(&a);
      printf("| %6zu | Ошибка выделения памяти                    |\n", i);
      printf("+--------+------------+---------------+----------------+\n");
      break;
    }

    if (alloc_matrix(&c) != 0) {
      free_matrix(&a);
      free_matrix(&b);
      printf("| %6zu | Ошибка выделения памяти                    |\n", i);
      printf("+--------+------------+---------------+----------------+\n");
      break;
    }

    fill_matrix_random(&a, 100);
    fill_matrix_random(&b, 100);

    double total_time_standard = 0;
    double total_time_winograd = 0;
    double total_time_optimized = 0;

    for (int i = 0; i < ITER_NUM; i++) {
      clock_t start = clock();
      multiply_matrix_standart_algo(&c, a, b);
      clock_t end = clock();
      total_time_standard += (((double)(end - start)) / CLOCKS_PER_SEC) * 1000;
    }

    for (int iter = 0; iter < ITER_NUM; iter++) {
      clock_t start = clock();
      winograd_algo(&c, a, b);
      clock_t end = clock();
      total_time_winograd += (((double)(end - start)) / CLOCKS_PER_SEC) * 1000;
    }

    for (int iter = 0; iter < ITER_NUM; iter++) {
      clock_t start = clock();
      optimized_winograd_algo(&c, a, b);
      clock_t end = clock();
      total_time_optimized += (((double)(end - start)) / CLOCKS_PER_SEC) * 1000;
    }

    double avg_standard = total_time_standard / ITER_NUM;
    double avg_winograd = total_time_winograd / ITER_NUM;
    double avg_optimized = total_time_optimized / ITER_NUM;

    printf("| %6zu | %10.5g | %13.5g | %14.5g |\n", i, avg_standard,
           avg_winograd, avg_optimized);

    fprintf(csv_file, "%zu;%.5g;%.5g;%.5g\n", i, avg_standard, avg_winograd,
            avg_optimized);

    free_matrix(&a);
    free_matrix(&b);
    free_matrix(&c);
  }
  printf("+--------+------------+---------------+----------------+\n");
  fclose(csv_file);
}
