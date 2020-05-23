#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "uvwasi.h"

int main(void) {
  uvwasi_t uvwasi;
  uvwasi_options_t init_options;

  uvwasi_options_init(&init_options);
  assert(init_options.in == 0);
  assert(init_options.out == 1);
  assert(init_options.err == 2);
  assert(init_options.fd_table_size == 3);
  assert(init_options.argc == 0);
  assert(init_options.argv == NULL);
  assert(init_options.envp == NULL);
  assert(init_options.preopenc == 0);
  assert(init_options.preopens == NULL);
  assert(init_options.allocator == NULL);

  assert(0 == uvwasi_init(&uvwasi, &init_options));
  /* Calling uvwasi_destroy() multiple times should be fine. */
  uvwasi_destroy(&uvwasi);
  uvwasi_destroy(&uvwasi);
  uvwasi_destroy(&uvwasi);

  return 0;
}
