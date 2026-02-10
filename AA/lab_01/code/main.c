#include "../../lab_02/code/cpu_time.h"
#include "matrix.h"

#define OK 0
#define CHOICE_ERROR 1
#define ROWS_INPUT_ERROR 2
#define COLUMNS_INPUT_ERROR 3
#define MATRIX_SCAN_ERROR 4
#define NOT_MULTIPLIABLE_ERROR 5
#define MATRIX_ALLOCATION_ERR 6
#define START_LEN_ERR 7
#define STOP_LEN_ERR 8
#define STEP_ERR 9

int main(void) {
  int choice;
  int rows, cols;

  printf("----------------Меню----------------------\n");
  printf("1) Стандартный алгоритм умножения матриц\n");
  printf("2) Алгоритм Винограда\n");
  printf("3) Алгоритм Винограда Оптимизированный\n");
  printf("4) Выполнить замеры процессорного времени\n");
  printf("0) Выйти\n");

  if (scanf("%d", &choice) != 1 || choice < 0 || choice > 4) {
    printf("Ошибка! Выберите один из пунктов меню.\n");
    return CHOICE_ERROR;
  }

  if (choice == 4) {
    size_t start_len, stop_len, step;

    printf("Введите параметры замеров.\n");
    printf("Начальный размер квадратных матриц:\n");
    if (scanf("%zu", &start_len) != 1) {
      printf("Ошибка! Начальный размер матриц должен быть целым "
             "неотрицательным числом\n");
      return START_LEN_ERR;
    }

    printf("Конечный размер квадратных матриц:\n");
    if (scanf("%zu", &stop_len) != 1) {
      printf("Ошибка! Начальный размер матриц должен быть целым "
             "неотрицательным числом\n");
      return STOP_LEN_ERR;
    }

    printf("Шаг:\n");
    if (scanf("%zu", &step) != 1) {
      printf("Ошибка! Шаг должен быть целым неотрицательным числом\n");
      return STEP_ERR;
    }

    measure_cpu_time(start_len, stop_len, step);
  } else if (choice != 0) {
    printf("Введите количество строк первой матрицы\n");
    if (scanf("%d", &rows) != 1 || rows <= 0) {
      printf("Ошибка! Количество строк матрицы должно быть больше нуля.\n");
      return ROWS_INPUT_ERROR;
    }

    printf("Введите количество столбцов первой матрицы\n");
    if (scanf("%d", &cols) != 1 || cols <= 0) {
      printf("Ошибка! Количество столбцов матрицы должно быть больше нуля.\n");
      return COLUMNS_INPUT_ERROR;
    }

    matrix_t matrix_a = {0};
    matrix_a.n = rows;
    matrix_a.m = cols;

    if (alloc_matrix(&matrix_a) != 0) {
      printf("Ошибка выделения памяти для матрицы\n");
      return MATRIX_ALLOCATION_ERR;
    }

    printf("Введите первую матрицу\n");
    if (scan_matrix(&matrix_a) != 0) {
      printf("Ошибка ввода матрицы!\n");
      free_matrix(&matrix_a);
      return MATRIX_SCAN_ERROR;
    }

    printf("Введите количество строк второй матрицы\n");
    if (scanf("%d", &rows) != 1 || rows <= 0) {
      printf("Ошибка! Количество строк матрицы должно быть больше нуля.\n");
      free_matrix(&matrix_a);
      return ROWS_INPUT_ERROR;
    }

    printf("Введите количество столбцов второй матрицы\n");
    if (scanf("%d", &cols) != 1 || cols <= 0) {
      printf("Ошибка! Количество столбцов матрицы должно быть больше нуля.\n");
      free_matrix(&matrix_a);
      return COLUMNS_INPUT_ERROR;
    }

    matrix_t matrix_b = {0};
    matrix_b.n = rows;
    matrix_b.m = cols;
    if (alloc_matrix(&matrix_b) != 0) {
      printf("Ошибка выделения памяти для матрицы\n");
      free_matrix(&matrix_a);
      return MATRIX_ALLOCATION_ERR;
    }

    printf("Введите вторую матрицу\n");
    if (scan_matrix(&matrix_b) != 0) {
      printf("Ошибка ввода матрицы!\n");
      free_matrix(&matrix_a);
      free_matrix(&matrix_b);
      return MATRIX_SCAN_ERROR;
    }

    if (matrix_a.m != matrix_b.n) {
      printf("Ошибка! Умножение матриц невозможно. Кол-во столбцов первой "
             "матрицы должно быть равно количеству строк второй.\n");
      free_matrix(&matrix_a);
      free_matrix(&matrix_b);
      return NOT_MULTIPLIABLE_ERROR;
    }

    matrix_t result = {0};
    result.n = matrix_a.n;
    result.m = matrix_b.m;
    if (alloc_matrix(&result) != 0) {
      printf("Ошибка выделения памяти для матрицы\n");
      free_matrix(&matrix_a);
      free_matrix(&matrix_b);
      return MATRIX_ALLOCATION_ERR;
    }

    switch (choice) {
    case 1:
      multiply_matrix_standart_algo(&result, matrix_a, matrix_b);
      printf("Результат умножения:\n");
      print_matrix(result);
      break;

    case 2:
      winograd_algo(&result, matrix_a, matrix_b);
      printf("Результат умножения:\n");
      print_matrix(result);
      break;

    case 3:
      optimized_winograd_algo(&result, matrix_a, matrix_b);
      printf("Результат умножения:\n");
      print_matrix(result);
      break;
    }

    free_matrix(&matrix_a);
    free_matrix(&matrix_b);
    free_matrix(&result);
  }

  return OK;
}
