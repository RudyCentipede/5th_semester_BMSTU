#include "check_matrix.h"

int main(void) {
  int fail_std = 0, fail_wino = 0, fail_owino = 0;

  Suite *std, *wino, *owino;
  SRunner *runner;

  std = standart_algo_suite();
  runner = srunner_create(std);
  srunner_run_all(runner, CK_VERBOSE);
  fail_std = srunner_ntests_failed(runner);
  srunner_free(runner);

  wino = winograd_suite();
  runner = srunner_create(wino);
  srunner_run_all(runner, CK_VERBOSE);
  fail_wino = srunner_ntests_failed(runner);
  srunner_free(runner);

  owino = optimized_winograd_suite();
  runner = srunner_create(owino);
  srunner_run_all(runner, CK_VERBOSE);
  fail_owino = srunner_ntests_failed(runner);
  srunner_free(runner);

  return (fail_std + fail_wino + fail_owino) != 0;
}
