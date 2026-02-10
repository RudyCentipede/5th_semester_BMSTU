#include "matrix.h"

void free_matrix(matrix_t *matrix) {
  for (size_t i = 0; i < matrix->n; i++)
    if (matrix->data[i])
      free(matrix->data[i]);
  if (matrix->data)
    free(matrix->data);
}

int alloc_matrix(matrix_t *matrix) {
  matrix->data = calloc(matrix->n, sizeof(int *));
  if (matrix->data == NULL)
    return 1;

  for (size_t i = 0; i < matrix->n; i++) {
    matrix->data[i] = calloc(matrix->m, sizeof(int));
    if (matrix->data[i] == NULL) {
      free_matrix(matrix);
      return 1;
    }
  }
  return 0;
}

int scan_matrix(matrix_t *matrix) {
  for (size_t i = 0; i < matrix->n; i++)
    for (size_t j = 0; j < matrix->m; j++)
      if (scanf("%d", &matrix->data[i][j]) != 1)
        return 1;
  return 0;
}

void print_matrix(matrix_t matrix) {
  for (size_t i = 0; i < matrix.n; i++) {
    for (size_t j = 0; j < matrix.m; j++)
      printf("%d ", matrix.data[i][j]);
    printf("\n");
  }
}

void multiply_matrix_standart_algo(matrix_t *res, matrix_t mtr1,
                                   matrix_t mtr2) {
  size_t n = mtr1.n;
  size_t m = mtr1.m;
  size_t q = mtr2.m;
  int **a = mtr1.data;
  int **b = mtr2.data;
  int **c = res->data;

  for (size_t i = 0; i < n; i++)
    for (size_t j = 0; j < q; j++)
      for (size_t k = 0; k < m; k++)
        c[i][j] += a[i][k] * b[k][j];
}

void winograd_algo(matrix_t *res, matrix_t mtr1, matrix_t mtr2) {
  size_t n = mtr1.n;
  size_t m = mtr1.m;
  size_t q = mtr2.m;
  int **a = mtr1.data;
  int **b = mtr2.data;
  int **c = res->data;

  int *mulH = calloc(n, sizeof(int));
  int *mulV = calloc(m, sizeof(int));

  for (size_t i = 0; i < n; i++)
    for (size_t j = 0; j < m / 2; j++)
      mulH[i] = mulH[i] + a[i][2 * j] * a[i][2 * j + 1];

  for (size_t i = 0; i < q; i++)
    for (size_t j = 0; j < m / 2; j++)
      mulV[i] = mulV[i] + b[2 * j][i] * b[2 * j + 1][i];

  for (size_t i = 0; i < n; i++)
    for (size_t j = 0; j < q; j++) {
      c[i][j] = -mulH[i] - mulV[j];
      for (size_t k = 0; k < m / 2; k++)
        c[i][j] = c[i][j] + (a[i][2 * k + 1] + b[2 * k][j]) * (a[i][2 * k] + b[2 * k + 1][j]);
    }

  if (m % 2 == 1)
    for (size_t i = 0; i < n; i++)
      for (size_t j = 0; j < q; j++)
        c[i][j] = c[i][j] + a[i][m - 1] * b[m - 1][j];

  free(mulH);
  free(mulV);
}

void optimized_winograd_algo(matrix_t *res, matrix_t mtr1, matrix_t mtr2) {
  size_t n = mtr1.n;
  size_t m = mtr1.m;
  size_t q = mtr2.m;
  int **a = mtr1.data;
  int **b = mtr2.data;
  int **c = res->data;

  int *mulH = calloc(n, sizeof(int));
  int *mulV = calloc(m, sizeof(int));

  size_t half_m = m / 2;

  for (size_t i = 0; i < n; i++) {
    int buf = 0;
    for (size_t j = 0; j < half_m; j++)
      buf = buf + a[i][2 * j] * a[i][2 * j + 1];
    mulH[i] = buf;
  }

  for (size_t i = 0; i < q; i++) {
    int buf = 0;
    for (size_t j = 0; j < half_m; j++)
      buf = buf + b[2 * j][i] * b[2 * j + 1][i];
    mulV[i] = buf;
  }

  for (size_t i = 0; i < n; i++) {
    int *row = a[i];
    int buf_mulH = mulH[i];
    for (size_t j = 0; j < q; j++) {
      int buf = -buf_mulH - mulV[j];
      for (size_t k = 0; k < half_m; k++) {
        size_t kk = 2 * k;
        buf = buf + (row[kk + 1] + b[kk][j]) * (row[kk] + b[kk + 1][j]);
      }
      c[i][j] = buf;
    }
  }

  if (m % 2 == 1) {
    size_t last = m - 1;
    for (size_t i = 0; i < n; i++) {
      int last_val = a[i][last];
      int *row = c[i];
      for (size_t j = 0; j < q; j++)
        row[j] = row[j] + last_val * b[last][j];
      c[i] = row;
    }
  }

  free(mulH);
  free(mulV);
}
