#include "check_algorithms.h"

START_TEST(test_iterative_empty_sequence) {
  int sequence[] = {2};
  size_t result = count_ones_iterative(sequence);
  ck_assert_uint_eq(result, 0);
}
END_TEST

START_TEST(test_iterative_all_zeros) {
  int sequence[] = {0, 0, 0, 0, 2};
  size_t result = count_ones_iterative(sequence);
  ck_assert_uint_eq(result, 0);
}
END_TEST

START_TEST(test_iterative_all_ones) {
  int sequence[] = {1, 1, 1, 1, 2};
  size_t result = count_ones_iterative(sequence);
  ck_assert_uint_eq(result, 4);
}
END_TEST

START_TEST(test_recursive_alternating) {
  int sequence[] = {1, 0, 1, 0, 1, 0, 1, 0, 2};
  size_t result = count_ones_iterative(sequence);
  ck_assert_uint_eq(result, 4);
}
END_TEST

START_TEST(test_iterative_single_one) {
  int sequence[] = {1, 2};
  size_t result = count_ones_iterative(sequence);
  ck_assert_uint_eq(result, 1);
}
END_TEST

START_TEST(test_iterative_single_zero) {
  int sequence[] = {0, 2};
  size_t result = count_ones_iterative(sequence);
  ck_assert_uint_eq(result, 0);
}
END_TEST

Suite *iterative_suite(void) {
  Suite *s = suite_create("count_ones_iterative");
  TCase *tc_core = tcase_create("Core");

  tcase_add_test(tc_core, test_iterative_empty_sequence);
  tcase_add_test(tc_core, test_iterative_all_zeros);
  tcase_add_test(tc_core, test_iterative_all_ones);
  tcase_add_test(tc_core, test_recursive_alternating);
  tcase_add_test(tc_core, test_iterative_single_one);
  tcase_add_test(tc_core, test_iterative_single_zero);

  suite_add_tcase(s, tc_core);
  return s;
}