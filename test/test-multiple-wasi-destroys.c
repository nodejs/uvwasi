#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "uvwasi.h"

int main(void) {
  uvwasi_t uvwasi;
  uvwasi_options_t init_options;
  uvwasi_errno_t err;

  init_options.in = 0;
  init_options.out = 1;
  init_options.err = 2;
  init_options.fd_table_size = 3;
  init_options.argc = 0;
  init_options.argv = NULL;
  init_options.envp = NULL;
  init_options.preopenc = 0;
  init_options.preopens = NULL;
  init_options.allocator = NULL;

  assert(0 == uvwasi_init(&uvwasi, &init_options));
  /* Calling uvwasi_destroy() multiple times should be fine. */
  uvwasi_destroy(&uvwasi);
  uvwasi_destroy(&uvwasi);
  uvwasi_destroy(&uvwasi);

  return 0;
}
