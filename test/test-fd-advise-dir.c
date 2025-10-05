#include <assert.h>
#include <stdlib.h>

#include "uv.h"
#include "uvwasi.h"
#include "test-common.h"

#define TEST_TMP_DIR "./out/tmp"
#define TEST_PATH_ADVISE TEST_TMP_DIR "/test_fd_advise_dir"

int main(void) {
#if !defined(_WIN32)
  uvwasi_t uvwasi;
  uvwasi_options_t init_options;
  uv_fs_t req;
  uvwasi_errno_t err;
  int r;

  setup_test_environment();

  r = uv_fs_mkdir(NULL, &req, TEST_TMP_DIR, 0777, NULL);
  uv_fs_req_cleanup(&req);
  assert(r == 0 || r == UV_EEXIST);

  r = uv_fs_mkdir(NULL, &req, TEST_PATH_ADVISE, 0777, NULL);
  uv_fs_req_cleanup(&req);
  assert(r == 0 || r == UV_EEXIST);

  uvwasi_options_init(&init_options);
  init_options.preopenc = 1;
  init_options.preopens = calloc(1, sizeof(uvwasi_preopen_t));
  init_options.preopens[0].mapped_path = "/var";
  init_options.preopens[0].real_path = TEST_PATH_ADVISE;

  err = uvwasi_init(&uvwasi, &init_options);
  assert(err == 0);

  err = uvwasi_fd_advise(&uvwasi, 3, 10, 20, UVWASI_ADVICE_DONTNEED);
  assert(err == UVWASI_EBADF);

  uvwasi_destroy(&uvwasi);
  free(init_options.preopens);
#endif /* !defined(_WIN32) */
  return 0;
}
