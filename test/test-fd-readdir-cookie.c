#include "test-common.h"
#include "uv.h"
#include "uvwasi.h"
#include "wasi_serdes.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_TMP_DIR "./out/tmp"
#define TEST_PATH_READDIR TEST_TMP_DIR "/test_readdir_cookie"

#if !defined(_WIN32) && !defined(__ANDROID__)
static void touch_file(const char *name) {
  uv_fs_t req;
  int r;

  r = uv_fs_open(NULL, &req, name, O_WRONLY | O_CREAT | O_TRUNC,
                 S_IWUSR | S_IRUSR, NULL);
  uv_fs_req_cleanup(&req);
  assert(r >= 0);
  r = uv_fs_close(NULL, &req, r, NULL);
  uv_fs_req_cleanup(&req);
  assert(r == 0);
}
#endif /* !defined(_WIN32) && !defined(__ANDROID__) */

/*
 * This is a test case for https://github.com/nodejs/node/issues/47193.
 */

int main(void) {
#if !defined(_WIN32) && !defined(__ANDROID__)
  uvwasi_t uvwasi;
  uvwasi_options_t init_options;
  uvwasi_dircookie_t cookie;
  uvwasi_dirent_t dirent;
  uvwasi_size_t buf_size;
  uvwasi_size_t buf_used;
  uvwasi_errno_t err;
  uv_fs_t req;
  uvwasi_fd_t tmp_fd = 3;
  char buf[64];
  int r;
  int cnt;

  setup_test_environment();

  r = uv_fs_mkdir(NULL, &req, TEST_TMP_DIR, 0777, NULL);
  uv_fs_req_cleanup(&req);
  assert(r == 0 || r == UV_EEXIST);

  r = uv_fs_mkdir(NULL, &req, TEST_PATH_READDIR, 0777, NULL);
  uv_fs_req_cleanup(&req);
  assert(r == 0 || r == UV_EEXIST);

  for (int i = 0; i < 10; i++) {
    const char *format = TEST_PATH_READDIR "/test_file_"
                                           "%d";
    int len = strlen(format) + 3;
    char file_name[len];
    snprintf(file_name, len, format, i);
    touch_file(file_name);
  }

  uvwasi_options_init(&init_options);
  init_options.preopenc = 1;
  init_options.preopens = calloc(1, sizeof(uvwasi_preopen_t));
  init_options.preopens[0].mapped_path = "/var";
  init_options.preopens[0].real_path = TEST_PATH_READDIR;

  err = uvwasi_init(&uvwasi, &init_options);
  assert(err == 0);

  buf_size = 64;
  memset(buf, 0, buf_size);
  buf_used = -1;
  cookie = UVWASI_DIRCOOKIE_START;
  cnt = 0;

  // For simplicity, we read entries one by one
  while (buf_used == -1 || buf_used == buf_size) {
    memset(buf, 0, buf_size);
    err = uvwasi_fd_readdir(&uvwasi, tmp_fd, &buf, buf_size, cookie, &buf_used);
    uvwasi_serdes_read_dirent_t(buf, 0, &dirent);
    assert(err == UVWASI_ESUCCESS);

    cookie = dirent.d_next;
    cnt += 1;

    // There are only 10 files
    assert(cnt <= 10);
  }

  assert(cnt == 10);
  uvwasi_destroy(&uvwasi);
  free(init_options.preopens);

#endif /* !defined(_WIN32) && !defined(__ANDROID__) */
  return 0;
}
