#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "uvwasi.h"
#include "uv.h"

#define TEST_TMP_DIR "./out/tmp"

int main(void) {
  const char* path = "./path-open-create-file.txt";
  const char* linkname = "./symlink.txt";
  uvwasi_fd_t fd;
  uvwasi_t uvwasi;
  uvwasi_options_t init_options;
  uvwasi_rights_t fs_rights_base;
  uvwasi_ciovec_t* ciovecs;
  uvwasi_iovec_t* iovecs;
  uvwasi_filestat_t stats;
  uvwasi_filestat_t stats2;
  uvwasi_filesize_t seek_position;
  uvwasi_errno_t err;
  uv_fs_t req;
  void* buf;
  uvwasi_size_t ciovec_size;
  uvwasi_size_t iovec_size;
  uvwasi_size_t nio;
  uvwasi_size_t i;
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

  /* Create a file. */
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

  /* Set the size of the file to zero. */
  err = uvwasi_fd_filestat_set_size(&uvwasi, fd, 0);
  assert(err == 0);

  /* Stat the file and verify the size. */
  err = uvwasi_fd_filestat_get(&uvwasi, fd, &stats);
  assert(err == 0);
  assert(stats.st_dev > 0);
  assert(stats.st_ino > 0);
  assert(stats.st_nlink == 1);
  assert(stats.st_size == 0);
  assert(stats.st_filetype == UVWASI_FILETYPE_REGULAR_FILE);
  assert(stats.st_atim > 0);
  assert(stats.st_mtim > 0);
  assert(stats.st_ctim > 0);

  /* Resize the file to 1024 bytes. */
  err = uvwasi_fd_filestat_set_size(&uvwasi, fd, 1024);
  assert(err == 0);

  /* Stat the file again to verify the changed size. */
  err = uvwasi_fd_filestat_get(&uvwasi, fd, &stats);
  assert(err == 0);
  assert(stats.st_size == 1024);

  /* Reset the file size to zero again. */
  err = uvwasi_fd_filestat_set_size(&uvwasi, fd, 0);
  assert(err == 0);
  err = uvwasi_fd_filestat_get(&uvwasi, fd, &stats);
  assert(err == 0);
  assert(stats.st_size == 0);

  /* Write data to the file. */
  ciovec_size = 2;
  nio = 0;
  ciovecs = calloc(ciovec_size, sizeof(*ciovecs));
  assert(ciovecs != NULL);
  for (i = 0; i < ciovec_size; ++i) {
    ciovecs[i].buf_len = 4;
    buf = malloc(ciovecs[i].buf_len);
    assert(buf != NULL);
    memset(buf, i, ciovecs[i].buf_len);
    ciovecs[i].buf = buf;
  }
  err = uvwasi_fd_write(&uvwasi, fd, ciovecs, ciovec_size, &nio);
  assert(err == 0);
  assert(nio == 8);

  /* Seek back to the beginning of the file. */
  err = uvwasi_fd_seek(&uvwasi, fd, 0, UVWASI_WHENCE_SET, &seek_position);
  assert(err == 0);
  assert(seek_position == 0);

  /* Read the data back from the file. */
  iovec_size = 2;
  iovecs = calloc(iovec_size, sizeof(*iovecs));
  assert(iovecs != NULL);
  nio = 0;
  for (i = 0; i < iovec_size; ++i) {
    iovecs[i].buf_len = 4;
    iovecs[i].buf = malloc(iovecs[i].buf_len);
    assert(iovecs[i].buf != NULL);
  }

  err = uvwasi_fd_read(&uvwasi, fd, iovecs, iovec_size, &nio);
  assert(err == 0);
  assert(nio == 8);

  /* Tell the position of the fd. */
  err = uvwasi_fd_tell(&uvwasi, fd, &seek_position);
  assert(err == 0);
  assert(seek_position == nio);

  /* Sync the fd. There isn't a great way to test these calls though. */
  err = uvwasi_fd_sync(&uvwasi, fd);
  assert(err == 0);
  err = uvwasi_fd_datasync(&uvwasi, fd);
  assert(err == 0);

  /* Close the file. */
  err = uvwasi_fd_close(&uvwasi, fd);
  assert(err == 0);

  /* Stat the file by name. */
  err = uvwasi_path_filestat_get(&uvwasi,
                                 3,
                                 0,
                                 path,
                                 strlen(path) + 1,
                                 &stats2);
  assert(err == 0);
  assert(stats2.st_dev == stats.st_dev);
  assert(stats2.st_ino == stats.st_ino);
  assert(stats2.st_nlink == 1);
  assert(stats2.st_size == 8);
  assert(stats2.st_filetype == UVWASI_FILETYPE_REGULAR_FILE);
  assert(stats2.st_atim > 0);
  assert(stats2.st_mtim > 0);
  assert(stats2.st_ctim > 0);

  /* Create symlink */
  err = uvwasi_path_symlink(&uvwasi,
                            path,
                            strlen(path) + 1,
                            3,
                            linkname,
                            strlen(linkname)+1);
  assert(err == 0);

  /* Stat of symlink with UVWASI_LOOKUP_SYMLINK_FOLLOW should yield results
   * idential the stat'ing the target file */
  err = uvwasi_path_filestat_get(&uvwasi,
                                 3,
                                 UVWASI_LOOKUP_SYMLINK_FOLLOW,
                                 linkname,
                                 strlen(path) + 1,
                                 &stats2);
  assert(err == 0);
  assert(stats2.st_dev == stats.st_dev);
  assert(stats2.st_ino == stats.st_ino);
  assert(stats2.st_nlink == 1);
  assert(stats2.st_size == 8);
  assert(stats2.st_filetype == UVWASI_FILETYPE_REGULAR_FILE);
  assert(stats2.st_atim > 0);
  assert(stats2.st_mtim > 0);
  assert(stats2.st_ctim > 0);

  /* If UVWASI_LOOKUP_SYMLINK_FOLLOW is not set we stat the link itself */
  err = uvwasi_path_filestat_get(&uvwasi,
                                 3,
                                 0,
                                 linkname,
                                 strlen(path) + 1,
                                 &stats2);
  assert(err == 0);
  assert(stats2.st_dev == stats.st_dev);
  assert(stats2.st_ino != stats.st_ino);
  assert(stats2.st_nlink == 1);
  assert(stats2.st_filetype == UVWASI_FILETYPE_SYMBOLIC_LINK);
  assert(stats2.st_atim > 0);
  assert(stats2.st_mtim > 0);
  assert(stats2.st_ctim > 0);


  /* Unlink the files. */
  err = uvwasi_path_unlink_file(&uvwasi, 3, path, strlen(path) + 1);
  assert(err == 0);
  err = uvwasi_path_unlink_file(&uvwasi, 3, linkname, strlen(linkname) + 1);
  assert(err == 0);

  /* Clean things up. */
  uvwasi_destroy(&uvwasi);

  for (i = 0; i < ciovec_size; ++i) {
    buf = (void*) ciovecs[i].buf;
    free(buf);
  }

  free(ciovecs);

  for (i = 0; i < iovec_size; ++i)
    free(iovecs[i].buf);

  free(iovecs);
  free(init_options.preopens);

  return 0;
}
