#include "../include/fd_table.h"
#include "test_fd_table.h"
#include <check.h>

START_TEST(test_uvwasi_fd_table_init) {
  uvwasi_errno_t ret = uvwasi_fd_table_init(NULL, 10);
  ck_assert_msg(ret == UVWASI_EINVAL,
                "Expected UVWASI_EINVAL if fd_table_t is NULL");

  struct uvwasi_fd_table_t table;
  ret = uvwasi_fd_table_init(&table, 2);
  ck_assert_msg(ret == UVWASI_EINVAL,
                "Expected UVWASI_EINVAL but was was %d", ret);
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
