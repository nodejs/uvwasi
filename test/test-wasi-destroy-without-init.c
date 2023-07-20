#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "uvwasi.h"
#include "test-common.h"

int main(void) {
  setup_test_environment();

  uvwasi_t uvwasi = {0};
  uvwasi_destroy(&uvwasi);
  return 0;
}
