#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "uv.h"
#include "uvwasi.h"
#include "wasi_serdes.h"

#define TEST_TMP_DIR "./out/tmp"
#define TEST_PATH_READDIR TEST_TMP_DIR "/test_readdir"
#define TEST_PATH_FILE_1 TEST_PATH_READDIR "/test_file_1"
#define TEST_PATH_FILE_2 TEST_PATH_READDIR "/test_file_2"


#if !defined(_WIN32) && !defined(__ANDROID__)
static void touch_file(const char* name) {
  uv_fs_t req;
  int r;

  r = uv_fs_open(NULL,
                 &req,
                 name,
                 O_WRONLY | O_CREAT | O_TRUNC,
                 S_IWUSR | S_IRUSR,
                 NULL);
  uv_fs_req_cleanup(&req);
  assert(r >= 0);
  r = uv_fs_close(NULL, &req, r, NULL);
  uv_fs_req_cleanup(&req);
  assert(r == 0);
}
#endif /* !defined(_WIN32) && !defined(__ANDROID__) */


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
  char buf[1024];
  char* name;
  int r;

  r = uv_fs_mkdir(NULL, &req, TEST_TMP_DIR, 0777, NULL);
  uv_fs_req_cleanup(&req);
  assert(r == 0 || r == UV_EEXIST);

  r = uv_fs_mkdir(NULL, &req, TEST_PATH_READDIR, 0777, NULL);
  uv_fs_req_cleanup(&req);
  assert(r == 0 || r == UV_EEXIST);

  touch_file(TEST_PATH_FILE_1);
  touch_file(TEST_PATH_FILE_2);

  uvwasi_options_init(&init_options);
  init_options.preopenc = 1;
  init_options.preopens = calloc(1, sizeof(uvwasi_preopen_t));
  init_options.preopens[0].mapped_path = "/var";
  init_options.preopens[0].real_path = TEST_PATH_READDIR;

  err = uvwasi_init(&uvwasi, &init_options);
  assert(err == 0);

  buf_size = 1024;

  /* Verify uvwasi_fd_readdir() behavior with insufficient buffer space. */
  memset(buf, 0, buf_size);
  buf_used = 0;
  cookie = UVWASI_DIRCOOKIE_START;
  err = uvwasi_fd_readdir(&uvwasi,
                          3,
                          &buf,
                          UVWASI_SERDES_SIZE_dirent_t + 10,
                          cookie,
                          &buf_used);
  assert(err == 0);
  assert(buf_used == UVWASI_SERDES_SIZE_dirent_t + 10);
  uvwasi_serdes_read_dirent_t(buf, 0, &dirent);
  assert(dirent.d_ino == 0);
  assert(dirent.d_namlen == 11);
  assert(dirent.d_type == UVWASI_FILETYPE_REGULAR_FILE);
  name = &buf[UVWASI_SERDES_SIZE_dirent_t];
  assert(strcmp(name, "test_file_") == 0);

  /* Verify uvwasi_fd_readdir() happy path. */
  memset(buf, 0, buf_size);
  buf_used = 0;
  cookie = UVWASI_DIRCOOKIE_START;
  err = uvwasi_fd_readdir(&uvwasi, 3, &buf, buf_size, cookie, &buf_used);
  assert(err == 0);
  assert(buf_used == 2 * (UVWASI_SERDES_SIZE_dirent_t + 11));
  uvwasi_serdes_read_dirent_t(buf, 0, &dirent);
  assert(dirent.d_ino == 0);
  assert(dirent.d_namlen == 11);
  assert(dirent.d_type == UVWASI_FILETYPE_REGULAR_FILE);
  name = &buf[UVWASI_SERDES_SIZE_dirent_t];
  assert(strncmp(name, "test_file_1", 11) == 0 ||
         strncmp(name, "test_file_2", 11) == 0);
  uvwasi_serdes_read_dirent_t(buf, UVWASI_SERDES_SIZE_dirent_t + 11, &dirent);
  assert(dirent.d_ino == 0);
  assert(dirent.d_namlen == 11);
  assert(dirent.d_type == UVWASI_FILETYPE_REGULAR_FILE);
  name = &buf[(2 * UVWASI_SERDES_SIZE_dirent_t) + 11];
  assert(strncmp(name, "test_file_1", 11) == 0 ||
         strncmp(name, "test_file_2", 11) == 0);

  uvwasi_destroy(&uvwasi);
  free(init_options.preopens);
#endif /* !defined(_WIN32) && !defined(__ANDROID__) */
  return 0;
}
