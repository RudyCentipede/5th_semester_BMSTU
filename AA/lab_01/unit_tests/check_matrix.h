#ifndef CHECK_MATRIX
#define CHECK_MATRIX

#include "matrix.h"
#include <check.h>
#include <stdlib.h>

Suite *winograd_suite(void);

Suite *standart_algo_suite(void);

Suite *optimized_winograd_suite(void);

#endif
