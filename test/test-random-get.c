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

  uvwasi_options_init(&init_options);
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
