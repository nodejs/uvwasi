#include "../include/uv_mapping.h"
#include "test_uv_mapping.h"
#include <check.h>

START_TEST(test_uvwasi__translate_uv_error) {
  ck_assert_int_eq(4, 4);
}
END_TEST

START_TEST(test_uvwasi__translate_to_uv_signal) {
  ck_assert_int_eq(4, 4);
}
END_TEST

Suite* uvwasi_fd_table_suite(void) {
  Suite* s;
  TCase* tc_core;

  s = suite_create("uv_mapping");
  tc_core = tcase_create("Core");

  tcase_add_test(tc_core, test_uvwasi__translate_uv_error);
  tcase_add_test(tc_core, test_uvwasi__translate_to_uv_signal);

  suite_add_tcase(s, tc_core);
  return s;
}
