#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "uvwasi.h"

#define BUFFER_SIZE 1024

int main(void) {
  uvwasi_t uvwasi;
  uvwasi_options_t init_options;
  uvwasi_errno_t err;
  unsigned char* buf;
  int success;
  int i;

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

  err = uvwasi_init(&uvwasi, &init_options);
  assert(err == 0);

  buf = (unsigned char*) malloc(BUFFER_SIZE);
  assert(buf != NULL);
  memset(buf, 0, BUFFER_SIZE);

  for (i = 0; i < BUFFER_SIZE; i++)
    assert(buf[i] == 0);

  err = uvwasi_random_get(&uvwasi, buf, BUFFER_SIZE);
  assert(err == 0);

  success = 0;
  for (i = 0; i < BUFFER_SIZE; i++) {
    if (buf[i] != 0) {
      success = 1;
      break;
    }
  }

  assert(success == 1);
  free(buf);
  uvwasi_destroy(&uvwasi);
  return 0;
}
