#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "uvwasi.h"

#define TEST_TMP_DIR "./out/tmp"

int main(void) {
  const char* path = "./path-open-create-file.txt";
  uvwasi_fd_t fd;
  uvwasi_t uvwasi;
  uvwasi_options_t init_options;
  uvwasi_errno_t err;
  uv_fs_t req;
  int r;

  r = uv_fs_mkdir(NULL, &req, TEST_TMP_DIR, 0777, NULL);
  uv_fs_req_cleanup(&req);
  assert(r == 0 || r == UV_EEXIST);

  init_options.fd_table_size = 3;
  init_options.argc = 0;
  init_options.argv = NULL;
  init_options.envp = NULL;
  init_options.preopenc = 1;
  init_options.preopens = calloc(1, sizeof(uvwasi_preopen_t));
  init_options.preopens[0].mapped_path = "/var";
  init_options.preopens[0].real_path = TEST_TMP_DIR;

  err = uvwasi_init(&uvwasi, &init_options);
  assert(err == 0);

  /* Create a file. */
  err = uvwasi_path_open(&uvwasi,
                         3,
                         1,
                         path,
                         strlen(path) + 1,
                         UVWASI_O_CREAT,
                         264240830,
                         268435455,
                         0,
                         &fd);
  assert(err == 0);

  uvwasi_destroy(&uvwasi);
  return 0;
}
