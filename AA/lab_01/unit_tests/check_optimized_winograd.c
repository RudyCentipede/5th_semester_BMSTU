#include "check_matrix.h"

START_TEST(square_matrix) {
  matrix_t a = {0};
  matrix_t b = {0};
  matrix_t c = {0};
  a.n = 2;
  a.m = 2;
  alloc_matrix(&a);

  a.data[0][0] = 1;
  a.data[0][1] = 2;
  a.data[1][0] = 3;
  a.data[1][1] = 4;

  b.n = 2;
  b.m = 2;
  alloc_matrix(&b);

  b.data[0][0] = 10;
  b.data[0][1] = 20;
  b.data[1][0] = 30;
  b.data[1][1] = 40;

  c.n = 2;
  c.m = 2;
  alloc_matrix(&c);

  matrix_t res;
  res.n = 2;
  res.m = 2;
  alloc_matrix(&res);

  res.data[0][0] = 70;
  res.data[0][1] = 100;
  res.data[1][0] = 150;
  res.data[1][1] = 220;

  optimized_winograd_algo(&c, a, b);
  ck_assert_int_eq(c.n, res.n);
  ck_assert_int_eq(c.m, res.m);
  for (size_t i = 0; i < c.n; i++)
    for (size_t j = 0; j < c.m; j++)
      ck_assert_int_eq(c.data[i][j], res.data[i][j]);

  free_matrix(&a);
  free_matrix(&b);
  free_matrix(&c);
  free_matrix(&res);
}

END_TEST

START_TEST(rectangle_matrix) {
  matrix_t a = {0};
  matrix_t b = {0};
  matrix_t c = {0};
  a.n = 3;
  a.m = 2;
  alloc_matrix(&a);

  a.data[0][0] = 1;
  a.data[0][1] = 2;
  a.data[1][0] = 3;
  a.data[1][1] = 4;
  a.data[2][0] = 5;
  a.data[2][1] = 6;

  b.n = 2;
  b.m = 3;
  alloc_matrix(&b);

  b.data[0][0] = 1;
  b.data[0][1] = 2;
  b.data[0][2] = 3;
  b.data[1][0] = 4;
  b.data[1][1] = 5;
  b.data[1][2] = 6;

  c.n = 3;
  c.m = 3;
  alloc_matrix(&c);

  matrix_t res;
  res.n = 3;
  res.m = 3;
  alloc_matrix(&res);

  res.data[0][0] = 9;
  res.data[0][1] = 12;
  res.data[0][2] = 15;
  res.data[1][0] = 19;
  res.data[1][1] = 26;
  res.data[1][2] = 33;
  res.data[2][0] = 29;
  res.data[2][1] = 40;
  res.data[2][2] = 51;

  optimized_winograd_algo(&c, a, b);
  ck_assert_int_eq(c.n, res.n);
  ck_assert_int_eq(c.m, res.m);
  for (size_t i = 0; i < c.n; i++)
    for (size_t j = 0; j < c.m; j++)
      ck_assert_int_eq(c.data[i][j], res.data[i][j]);

  free_matrix(&a);
  free_matrix(&b);
  free_matrix(&c);
  free_matrix(&res);
}

END_TEST

Suite *optimized_winograd_suite(void) {
  Suite *s = suite_create("optimized_winograd_algo");
  TCase *case_pos, *case_neg;

  case_pos = tcase_create("POSITIVE");
  tcase_add_test(case_pos, square_matrix);
  tcase_add_test(case_pos, rectangle_matrix);

  suite_add_tcase(s, case_pos);

  return s;
}