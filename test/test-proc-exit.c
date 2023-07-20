#include "uvwasi.h"
#include "test-common.h"

int main(void) {
  setup_test_environment();

  /* uvwasi_proc_exit() is the only call that works with a NULL uvwasi_t. */
  uvwasi_proc_exit(NULL, 0);
  return 1;
}
