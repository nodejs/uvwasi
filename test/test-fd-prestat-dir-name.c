#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "uvwasi.h"
#include "uv.h"

#define TEST_TMP_DIR "./out/tmp"

int main(void) {
  uvwasi_t uvwasi;
  uvwasi_options_t init_options;
  uvwasi_prestat_t prestat;
  uvwasi_errno_t err;
  uvwasi_size_t prestat_buf_size;
  char* prestat_buf;
  uv_fs_t req;
  int r;

  r = uv_fs_mkdir(NULL, &req, TEST_TMP_DIR, 0777, NULL);
  uv_fs_req_cleanup(&req);
  assert(r == 0 || r == UV_EEXIST);

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

  /* Verify uvwasi_fd_prestat_get(). */
  err = uvwasi_fd_prestat_get(&uvwasi, 3, &prestat);
  assert(err == 0);
  assert(prestat.pr_type == UVWASI_PREOPENTYPE_DIR);
  assert(prestat.u.dir.pr_name_len ==
         strlen(init_options.preopens[0].mapped_path) + 1);

  /* Verify uvwasi_fd_prestat_dir_name(). */
  prestat_buf_size = prestat.u.dir.pr_name_len + 1;
  prestat_buf = malloc(prestat_buf_size);
  assert(prestat_buf != NULL);
  err = uvwasi_fd_prestat_dir_name(&uvwasi,
                                   3,
                                   prestat_buf,
                                   prestat_buf_size);
  assert(err == 0);
  assert(strcmp(prestat_buf, init_options.preopens[0].mapped_path) == 0);
  free(prestat_buf);
  free(init_options.preopens);
  uvwasi_destroy(&uvwasi);
  return 0;
}
