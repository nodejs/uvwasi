#include <stdio.h>
#include <stdlib.h>
// Include test headers
#include "test_uv_mapping.h"
#include "test_fd_table.h"
#include <check.h>

int main(void) {
  int no_failed = 0;
  Suite* s;
  SRunner* sr;

  s = suite_create("UVWASI");
  sr = srunner_create(s);

  // Add test suites
  srunner_add_suite(sr, uvwasi_mapping_suite());
  srunner_add_suite(sr, uvwasi_fd_table_suite());

  srunner_run_all(sr, CK_NORMAL);
  no_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (no_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
