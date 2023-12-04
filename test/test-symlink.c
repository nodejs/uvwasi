#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "uvwasi.h"
#include "uv.h"
#include "test-common.h"

#define TEST_TMP_DIR "./out/tmp"
#define BUFFER_SIZE 1024

int main(void) {
  const char* path = "./path-open-create-file.txt";
  const char* truncated_path = "./path-open-create-file";
  const char* linkname = "./symlink.txt";
  const char* truncated_linkname = "./symlink";
  uvwasi_t uvwasi;
  uvwasi_options_t init_options;
  uvwasi_errno_t err;
  uv_fs_t req;
  int r;
  char* buf;
  uvwasi_size_t bufused;

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

  buf = (char*) malloc(BUFFER_SIZE);
  assert(buf != NULL);
  memset(buf, 0, BUFFER_SIZE);

  err = uvwasi_path_symlink(&uvwasi,
                            path,
                            strlen(path),
                            3,
                            linkname,
                            strlen(linkname));
  assert(err == 0);

  err = uvwasi_path_readlink(&uvwasi,
                             3,
                             linkname,
                             strlen(linkname),
                             buf,
                             strlen(path) + 1,
                             &bufused);
  assert(err == 0);
  assert(bufused == strlen(path) + 1);
  assert(strcmp(buf, path) == 0);

  err = uvwasi_path_unlink_file(&uvwasi, 3, linkname, strlen(linkname));
  assert(err == 0);

  /* Symlink old path truncation */
  err = uvwasi_path_symlink(&uvwasi,
                            path,
                            strlen(truncated_path), /* Intentionally truncate path */
                            3,
                            linkname,
                            strlen(truncated_linkname));
  assert(err == 0);

  err = uvwasi_path_readlink(&uvwasi,
                             3,
                             truncated_linkname,
                             strlen(truncated_linkname),
                             buf,
                             strlen(truncated_path) + 1,
                             &bufused);
  assert(err == 0);
  assert(bufused == strlen(truncated_path) + 1);
  assert(strcmp(buf, truncated_path) == 0);

  err = uvwasi_path_unlink_file(&uvwasi, 3, truncated_linkname, strlen(truncated_linkname));
  assert(err == 0);

  free(buf);
  free(init_options.preopens);
  uvwasi_destroy(&uvwasi);

  return 0;
}
