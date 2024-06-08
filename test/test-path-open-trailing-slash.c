#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "uvwasi.h"
#include "uv.h"
#include "test-common.h"

#define TEST_TMP_DIR "./out/tmp"
#define TEST_FILE    "./out/tmp/test-path-open-trailing-slash.file"

int main(void) {
  const char* path = "test-path-open-trailing-slash.file/";
  uvwasi_t uvwasi;
  uvwasi_fd_t fd;
  uvwasi_options_t init_options;
  uvwasi_errno_t err;
  uv_fs_t req;
  int r;

  setup_test_environment();

  r = uv_fs_mkdir(NULL, &req, TEST_TMP_DIR, 0777, NULL);
  uv_fs_req_cleanup(&req);
  assert(r == 0 || r == UV_EEXIST);

  r = uv_fs_open(NULL, &req, TEST_FILE, UV_FS_O_CREAT | UV_FS_O_RDWR, 0777, NULL);
  uv_fs_req_cleanup(&req);
  assert(r >= 0);

  uvwasi_options_init(&init_options);
  init_options.preopenc = 1;
  init_options.preopens = calloc(1, sizeof(uvwasi_preopen_t));
  init_options.preopens[0].mapped_path = "/var";
  init_options.preopens[0].real_path = TEST_TMP_DIR;

  err = uvwasi_init(&uvwasi, &init_options);
  assert(err == 0);

  err = uvwasi_path_open(&uvwasi,
                         3,
                         0,
                         path,
                         strlen(path) + 1,
                         0,
                         0,
                         0,
                         0,
                         &fd);
  assert(err != UVWASI_ESUCCESS);

  uvwasi_destroy(&uvwasi);
  free(init_options.preopens);

  return 0;
}
