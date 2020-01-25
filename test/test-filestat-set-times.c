#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "uvwasi.h"
#include "uv.h"

#define TEST_TMP_DIR "./out/tmp"

static const uvwasi_timestamp_t ONE_SECOND = 1000000000;
static const uvwasi_timestamp_t TIME_THRESHOLD = 5000000000;  /* 5 seconds. */

static void check_in_threshold(uvwasi_timestamp_t first,
                               uvwasi_timestamp_t second) {
  if (second < first)
    assert(first - second < ONE_SECOND);
  else
    assert(second - first <= TIME_THRESHOLD);
}

int main(void) {
  const char* path = "./test-filestat-set-times.txt";
  const char* linkname = "./symlink.txt";
  uvwasi_fd_t fd;
  uvwasi_t uvwasi;
  uvwasi_options_t init_options;
  uvwasi_rights_t fs_rights_base;
  uvwasi_filestat_t stats;
  uvwasi_filestat_t stats2;
  uvwasi_errno_t err;
  size_t pathsize;
  size_t linksize;
  uv_fs_t req;
  int r;

  r = uv_fs_mkdir(NULL, &req, TEST_TMP_DIR, 0777, NULL);
  uv_fs_req_cleanup(&req);
  assert(r == 0 || r == UV_EEXIST);
  pathsize = strlen(path) + 1;
  linksize = strlen(linkname) + 1;

  uvwasi_options_init(&init_options);
  init_options.preopenc = 1;
  init_options.preopens = calloc(1, sizeof(uvwasi_preopen_t));
  init_options.preopens[0].mapped_path = "/var";
  init_options.preopens[0].real_path = TEST_TMP_DIR;

  err = uvwasi_init(&uvwasi, &init_options);
  free(init_options.preopens);
  assert(err == 0);

  /* Create a file. */
  fs_rights_base = UVWASI_RIGHT_FD_FILESTAT_GET |
                   UVWASI_RIGHT_FD_FILESTAT_SET_TIMES |
                   UVWASI_RIGHT_PATH_FILESTAT_SET_TIMES |
                   /* Windows fails with EPERM without this. */
                   UVWASI_RIGHT_FD_WRITE;
  err = uvwasi_path_open(&uvwasi,
                         3,
                         1,
                         path,
                         pathsize,
                         UVWASI_O_CREATE,
                         fs_rights_base,
                         0,
                         0,
                         0,
                         &fd);
  assert(err == 0);

  /* Create a symlink. */
  err = uvwasi_path_symlink(&uvwasi, path, pathsize, 3, linkname, linksize);
  assert(err == 0);

  /* Stat the file to get initial times. */
  err = uvwasi_fd_filestat_get(&uvwasi, fd, &stats);
  assert(err == 0);
  assert(stats.st_filetype == UVWASI_FILETYPE_REGULAR_FILE);
  assert(stats.st_atim > 0);
  assert(stats.st_mtim > 0);
  assert(stats.st_ctim > 0);

  /* Test setting file times by file descriptor. */

  /* Set the file times to a specific time. */
  err = uvwasi_fd_filestat_set_times(&uvwasi,
                                     fd,
                                     1,
                                     ONE_SECOND,
                                     UVWASI_FILESTAT_SET_ATIM |
                                     UVWASI_FILESTAT_SET_MTIM);
  /* uvwasi_fd_filestat_set_times() uses uv_fs_futime(), which can return ENOSYS
     on AIX < 7.1. In that case, skip the rest of the test. */
  if (err == UVWASI_ENOSYS)
    goto exit;

  assert(err == 0);

  /* Get the new file times. */
  err = uvwasi_fd_filestat_get(&uvwasi, fd, &stats2);
  assert(err == 0);
  assert(stats2.st_filetype == UVWASI_FILETYPE_REGULAR_FILE);
  assert(stats2.st_atim == 0);
  assert(stats2.st_mtim == ONE_SECOND);
  assert(stats2.st_ctim > 0);

  /* Set the file times, but omit both values. */
  err = uvwasi_fd_filestat_set_times(&uvwasi,
                                     fd,
                                     ONE_SECOND * 5,
                                     ONE_SECOND * 9,
                                     0);

  /* Get the new file times - they should be the same as before. */
  err = uvwasi_fd_filestat_get(&uvwasi, fd, &stats2);
  assert(err == 0);
  assert(stats2.st_filetype == UVWASI_FILETYPE_REGULAR_FILE);
  assert(stats2.st_atim == 0);
  assert(stats2.st_mtim == ONE_SECOND);
  assert(stats2.st_ctim > 0);

  /* Set the file times to the current time. */
  err = uvwasi_fd_filestat_set_times(&uvwasi,
                                     fd,
                                     0,
                                     0,
                                     UVWASI_FILESTAT_SET_ATIM_NOW |
                                     UVWASI_FILESTAT_SET_MTIM_NOW);
  assert(err == 0);

  /* Get the new file times. */
  err = uvwasi_fd_filestat_get(&uvwasi, fd, &stats2);
  assert(err == 0);
  assert(stats2.st_filetype == UVWASI_FILETYPE_REGULAR_FILE);
  check_in_threshold(stats.st_atim, stats2.st_atim);
  check_in_threshold(stats.st_mtim, stats2.st_mtim);
  assert(stats2.st_ctim > 0);

  /* Close the file. */
  err = uvwasi_fd_close(&uvwasi, fd);
  assert(err == 0);

  /* Test setting file times by path. */

  /* Set the file times to a specific time. */
  err = uvwasi_path_filestat_set_times(&uvwasi,
                                       3,
                                       0,
                                       path,
                                       pathsize,
                                       ONE_SECOND + 1,
                                       ONE_SECOND * 3,
                                       UVWASI_FILESTAT_SET_ATIM |
                                       UVWASI_FILESTAT_SET_MTIM);
  /* uvwasi_path_filestat_set_times() uses uv_fs_lutime(), which can return
     ENOSYS on AIX < 7.1 and z/OS. In that case, skip the rest of the test. */
  if (err == UVWASI_ENOSYS)
    goto exit;

  assert(err == 0);

  /* Get the new file times. */
  err = uvwasi_path_filestat_get(&uvwasi, 3, 0, path, pathsize, &stats2);
  assert(err == 0);
  assert(stats2.st_filetype == UVWASI_FILETYPE_REGULAR_FILE);
  assert(stats2.st_atim == ONE_SECOND);
  assert(stats2.st_mtim == ONE_SECOND * 3);
  assert(stats2.st_ctim > 0);

  /* If UVWASI_LOOKUP_SYMLINK_FOLLOW is not set update the link itself. */
  err = uvwasi_path_filestat_set_times(&uvwasi,
                                       3,
                                       0,
                                       linkname,
                                       linksize,
                                       ONE_SECOND * 5,
                                       ONE_SECOND * 7,
                                       UVWASI_FILESTAT_SET_ATIM |
                                       UVWASI_FILESTAT_SET_MTIM);
  assert(err == 0);

  /* Get the new file times. */
  err = uvwasi_path_filestat_get(&uvwasi, 3, 0, linkname, linksize, &stats2);
  assert(err == 0);
  assert(stats2.st_filetype == UVWASI_FILETYPE_SYMBOLIC_LINK);
  assert(stats2.st_atim == ONE_SECOND * 5);
  assert(stats2.st_mtim == ONE_SECOND * 7);
  assert(stats2.st_ctim > 0);

  /* Updating a symlink with UVWASI_LOOKUP_SYMLINK_FOLLOW should yield results
     identical to updating the target file. */
  err = uvwasi_path_filestat_set_times(&uvwasi,
                                       3,
                                       UVWASI_LOOKUP_SYMLINK_FOLLOW,
                                       linkname,
                                       linksize,
                                       ONE_SECOND * 9,
                                       ONE_SECOND * 11,
                                       UVWASI_FILESTAT_SET_ATIM |
                                       UVWASI_FILESTAT_SET_MTIM);
  assert(err == 0);

  /* Get the new file times. */
  err = uvwasi_path_filestat_get(&uvwasi,
                                 3,
                                 UVWASI_LOOKUP_SYMLINK_FOLLOW,
                                 linkname,
                                 linksize,
                                 &stats2);
  assert(err == 0);
  assert(stats2.st_filetype == UVWASI_FILETYPE_REGULAR_FILE);
  assert(stats2.st_atim == ONE_SECOND * 9);
  assert(stats2.st_mtim == ONE_SECOND * 11);
  assert(stats2.st_ctim > 0);

exit:
  /* Unlink the files. */
  err = uvwasi_path_unlink_file(&uvwasi, 3, path, pathsize);
  assert(err == 0);
  err = uvwasi_path_unlink_file(&uvwasi, 3, linkname, linksize);
  assert(err == 0);

  /* Clean things up. */
  uvwasi_destroy(&uvwasi);

  return 0;
}
