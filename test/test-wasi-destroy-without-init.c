#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "uvwasi.h"

int main(void) {
  uvwasi_t uvwasi = {0};
  uvwasi_destroy(&uvwasi);
  return 0;
}
