#include <assert.h>
#include "uvwasi.h"

int main(void) {
  /* TODO(cjihrig): This test is intended to be temporary. */
  assert(UVWASI_ENOTSUP == uvwasi_sock_shutdown(NULL, 0, 0));

  return 0;
}
