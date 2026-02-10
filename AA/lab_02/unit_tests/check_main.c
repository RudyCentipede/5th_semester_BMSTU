#include "check_algorithms.h"

int main(void) {
  int fail_iterative = 0, fail_recursive = 0;

  Suite *is, *rs;
  SRunner *runner;

  is = iterative_suite();
  runner = srunner_create(is);
  srunner_run_all(runner, CK_NORMAL);
  fail_iterative = srunner_ntests_failed(runner);
  srunner_free(runner);

  rs = recursive_suite();
  runner = srunner_create(rs);
  srunner_run_all(runner, CK_NORMAL);
  fail_recursive = srunner_ntests_failed(runner);
  srunner_free(runner);

  return (fail_iterative + fail_recursive) != 0;
}