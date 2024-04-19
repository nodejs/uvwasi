#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "uvwasi.h"
#include "uv.h"
#include "test-common.h"

#define TEST_TMP_DIR "./out/tmp"

int main(void) {
  const char* path = "file.txt";
  uvwasi_t uvwasi;
  uvwasi_iovec_t* iovs;
  uvwasi_fd_t fd;
  uvwasi_options_t init_options;
  uvwasi_rights_t fs_rights_base;
  uvwasi_size_t nwritten;
  uvwasi_size_t iovs_len = 0;
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

  // Create a file.
  fs_rights_base = UVWASI_RIGHT_FD_DATASYNC |
                   UVWASI_RIGHT_FD_FILESTAT_GET |
                   UVWASI_RIGHT_FD_FILESTAT_SET_SIZE |
                   UVWASI_RIGHT_FD_READ |
                   UVWASI_RIGHT_FD_SEEK |
                   UVWASI_RIGHT_FD_SYNC |
                   UVWASI_RIGHT_FD_TELL |
                   UVWASI_RIGHT_FD_WRITE |
                   UVWASI_RIGHT_PATH_UNLINK_FILE;
  err = uvwasi_path_open(&uvwasi,
                         3,
                         1,
                         path,
                         strlen(path) + 1,
                         UVWASI_O_CREAT,
                         fs_rights_base,
                         0,
                         0,
                         &fd);
  assert(err == 0);

  iovs = calloc(iovs_len, sizeof(uvwasi_ciovec_t));
  assert(iovs != NULL);

  for (int i = 0; i < iovs_len; i++) {
    iovs[i].buf_len = 0;
    iovs[i].buf = malloc(0);
  }

  err = uvwasi_fd_read(&uvwasi, fd, iovs, iovs_len, &nwritten);
  assert(err == UVWASI_ESUCCESS);

  uvwasi_destroy(&uvwasi);

  for (int i = 0; i < iovs_len; i++) {
    free((void *) iovs[i].buf);
  }

  free(iovs);
  free(init_options.preopens);

  return 0;
}
