#include "../include/fd_table.h"
#include "test_fd_table.h"
#include <check.h>

START_TEST(test_uvwasi_fd_table_init) {
  ck_assert_int_eq(4, 4);
}
END_TEST

Suite* uvwasi_mapping_suite(void) {
  Suite* s;
  TCase* tc_core;

  s = suite_create("fd_table");
  tc_core = tcase_create("Core");

  tcase_add_test(tc_core, test_uvwasi_fd_table_init);

  suite_add_tcase(s, tc_core);
  return s;
}
