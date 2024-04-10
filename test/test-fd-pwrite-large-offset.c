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
  uvwasi_ciovec_t* ciovs;
  uvwasi_fd_t fd;
  uvwasi_filesize_t large_offset = (uint64_t) INT64_MAX + 1;
  uvwasi_options_t init_options;
  uvwasi_rights_t fs_rights_base;
  uvwasi_size_t nwritten;
  uvwasi_size_t ciovs_len = 1;
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

  ciovs = calloc(ciovs_len, sizeof(uvwasi_ciovec_t));
  assert(ciovs != NULL);

  for (int i = 0; i < ciovs_len; i++) {
    ciovs[i].buf_len = 1;
    ciovs[i].buf = malloc(1);
  }

  err = uvwasi_fd_pwrite(&uvwasi, fd, ciovs, ciovs_len, large_offset, &nwritten);
  assert(err == UVWASI_EINVAL);

  uvwasi_destroy(&uvwasi);

  for (int i = 0; i < ciovs_len; i++) {
    free((void *) ciovs[i].buf);
  }

  free(ciovs);
  free(init_options.preopens);

  return 0;
}
