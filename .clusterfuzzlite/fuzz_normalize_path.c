#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../src/path_resolver.h"

#define BUFFER_SIZE 128

char normalized_buffer[BUFFER_SIZE+1];

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  char *new_str = (char *)malloc(size + 1);
  if (new_str == NULL) {
    return 0;
  }
  memcpy(new_str, data, size);
  new_str[size] = '\0';

  memset(normalized_buffer, 0, BUFFER_SIZE);

  uvwasi__normalize_path(new_str, size, normalized_buffer, BUFFER_SIZE);

  free(new_str);
  return 0;
}
