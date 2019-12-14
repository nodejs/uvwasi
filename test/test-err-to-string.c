#include <assert.h>
#include <string.h>
#include "uvwasi.h"

int main(void) {
  /* Verify that a few uvwasi_embedder_err_code_to_string() calls work. */
  assert(0 == strcmp(uvwasi_embedder_err_code_to_string(UVWASI_E2BIG),
                     "UVWASI_E2BIG"));
  assert(0 == strcmp(uvwasi_embedder_err_code_to_string(UVWASI_EBADF),
                     "UVWASI_EBADF"));
  assert(0 == strcmp(uvwasi_embedder_err_code_to_string(UVWASI_ENOENT),
                     "UVWASI_ENOENT"));
  assert(0 == strcmp(uvwasi_embedder_err_code_to_string(UVWASI_ENOTCAPABLE),
                     "UVWASI_ENOTCAPABLE"));
  assert(0 == strcmp(uvwasi_embedder_err_code_to_string(UVWASI_ESUCCESS),
                     "UVWASI_ESUCCESS"));
  assert(0 == strcmp(uvwasi_embedder_err_code_to_string(34000),
                     "UVWASI_UNKNOWN_ERROR"));

  return 0;
}
