#ifndef MATRIX_H
#define MATRIX_H

#include <stdio.h>
#include <stdlib.h>

typedef struct {
  int **data;
  size_t n;
  size_t m;
} matrix_t;

void free_matrix(matrix_t *matrix);

int alloc_matrix(matrix_t *matrix);

int scan_matrix(matrix_t *matrix);

void print_matrix(matrix_t matrix);

void multiply_matrix_standart_algo(matrix_t *res, matrix_t mtr1, matrix_t mtr2);

void winograd_algo(matrix_t *res, matrix_t mtr1, matrix_t mtr2);

void optimized_winograd_algo(matrix_t *res, matrix_t mtr1, matrix_t mtr2);

#endif
