#include "test-common.h"
#include "uv.h"
#include "uvwasi.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define TEST_TMP_DIR "./out/tmp"

int main(void) {
#if !defined(_WIN32) && !defined(__ANDROID__)
  uvwasi_t uvwasi;
  uvwasi_options_t init_options;
  uvwasi_errno_t err;
  uv_fs_t req;
  int r;

  setup_test_environment();

  r = uv_fs_mkdir(NULL, &req, TEST_TMP_DIR, 0777, NULL);
  uv_fs_req_cleanup(&req);
  assert(r == 0 || r == UV_EEXIST);

  uvwasi_options_init(&init_options);
  init_options.preopenc = 1;
  init_options.preopens = calloc(1, sizeof(uvwasi_preopen_t));
  init_options.preopens[0].mapped_path = "/var";
  init_options.preopens[0].real_path = TEST_TMP_DIR;

  err = uvwasi_init(&uvwasi, &init_options);
  assert(err == 0);

  err = uvwasi_fd_filestat_set_times(&uvwasi, 3, 100, 200,
                                     UVWASI_FILESTAT_SET_ATIM |
                                         UVWASI_FILESTAT_SET_ATIM_NOW);
  assert(err == UVWASI_EINVAL);

  err = uvwasi_fd_filestat_set_times(&uvwasi, 3, 100, 200,
                                     UVWASI_FILESTAT_SET_MTIM |
                                         UVWASI_FILESTAT_SET_MTIM_NOW);
  assert(err == UVWASI_EINVAL);

  uvwasi_destroy(&uvwasi);
  free(init_options.preopens);
#endif /* !defined(_WIN32) && !defined(__ANDROID__) */
  return 0;
}
