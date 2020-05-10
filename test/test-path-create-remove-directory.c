#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "uvwasi.h"
#include "uv.h"

#define TEST_TMP_DIR "./out/tmp"
#define TEST_MKDIR_PATH TEST_TMP_DIR "/test_dir"

int main(void) {
  uvwasi_t uvwasi;
  uvwasi_options_t init_options;
  uvwasi_errno_t err;
  uv_fs_t req;
  int r;

  r = uv_fs_mkdir(NULL, &req, TEST_TMP_DIR, 0777, NULL);
  uv_fs_req_cleanup(&req);
  assert(r == 0 || r == UV_EEXIST);

  r = uv_fs_rmdir(NULL, &req, TEST_MKDIR_PATH, NULL);
  uv_fs_req_cleanup(&req);
  assert(r == 0 || r == UV_ENOENT);

  init_options.in = 0;
  init_options.out = 1;
  init_options.err = 2;
  init_options.fd_table_size = 3;
  init_options.argc = 0;
  init_options.argv = NULL;
  init_options.envp = NULL;
  init_options.preopenc = 1;
  init_options.preopens = calloc(1, sizeof(uvwasi_preopen_t));
  init_options.preopens[0].mapped_path = "/var";
  init_options.preopens[0].real_path = TEST_TMP_DIR;
  init_options.allocator = NULL;

  err = uvwasi_init(&uvwasi, &init_options);
  assert(err == 0);

  /* Verify uvwasi_path_create_directory() happy path. */
  err = uvwasi_path_create_directory(&uvwasi,
                                     3,
                                     "test_dir",
                                     strlen("test_dir") + 1);
  assert(err == 0);

  /* Verify uvwasi_path_create_directory() fails if directory already exists. */
  err = uvwasi_path_create_directory(&uvwasi,
                                     3,
                                     "test_dir",
                                     strlen("test_dir") + 1);
  assert(err == UVWASI_EEXIST);

  /* Verify uvwasi_path_create_directory() is sandboxed. */
  err = uvwasi_path_create_directory(&uvwasi,
                                     3,
                                     "../test_dir",
                                     strlen("../test_dir") + 1);
  assert(err == UVWASI_ENOTCAPABLE);

  /* Verify uvwasi_path_remove_directory() happy path. */
  err = uvwasi_path_remove_directory(&uvwasi,
                                     3,
                                     "test_dir",
                                     strlen("test_dir") + 1);
  assert(err == 0);

  /* Verify uvwasi_path_remove_directory() fails if directory is missing. */
  err = uvwasi_path_remove_directory(&uvwasi,
                                     3,
                                     "test_dir",
                                     strlen("test_dir") + 1);
  assert(err == UVWASI_ENOENT);

  /* Verify uvwasi_path_remove_directory() is sandboxed. */
  err = uvwasi_path_remove_directory(&uvwasi,
                                     3,
                                     "../test_dir",
                                     strlen("../test_dir") + 1);
  assert(err == UVWASI_ENOTCAPABLE);
  uvwasi_destroy(&uvwasi);
  free(init_options.preopens);
  return 0;
}
