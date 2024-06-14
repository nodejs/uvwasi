#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "uvwasi.h"
#include "uv.h"
#include "test-common.h"

#define TEST_TMP_DIR "./out/tmp"
#define LINK_CONTENT "/absolute"
#define TEST_FILE    "./out/tmp/test-path-symlink-absolute-path.link"

int main(void) {
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

  err = uvwasi_path_symlink(&uvwasi,
                            LINK_CONTENT,
                            strlen(LINK_CONTENT) + 1,
                            3,
                            TEST_FILE,
                            strlen(TEST_FILE) + 1);
  assert(err == UVWASI_EPERM);

  uvwasi_destroy(&uvwasi);
  free(init_options.preopens);

  return 0;
}
