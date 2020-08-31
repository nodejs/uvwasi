#include <assert.h>
#include <string.h>
#include "uvwasi.h"

#define CHECK(expr) assert(UVWASI_EBADF == (expr))

int main(void) {
  uvwasi_t uvw;
  uvwasi_options_t init_options;
  uvwasi_errno_t err;
  uvwasi_fdstat_t test_fdstat;
  uvwasi_filestat_t test_filestat;
  uvwasi_iovec_t test_iovec;
  uvwasi_ciovec_t test_ciovec;
  uvwasi_prestat_t test_prestat;
  uvwasi_filesize_t test_filesize;
  uvwasi_dircookie_t test_dircookie = 0;
  uvwasi_fd_t test_fd;
  char* test_str = "foo";
  uvwasi_size_t test_size = 5;
  void* test_void;

  test_void = (void*) &test_fdstat;

  uvwasi_options_init(&init_options);
  err = uvwasi_init(&uvw, &init_options);
  assert(err == 0);

  CHECK(uvwasi_fd_advise(&uvw, 100, 10, 20, UVWASI_ADVICE_DONTNEED));
  CHECK(uvwasi_fd_allocate(&uvw, 100, 10, 20));
  CHECK(uvwasi_fd_close(&uvw, 100));
  CHECK(uvwasi_fd_datasync(&uvw, 100));
  CHECK(uvwasi_fd_fdstat_get(&uvw, 100, &test_fdstat));
#ifndef _WIN32
  CHECK(uvwasi_fd_fdstat_set_flags(&uvw, 100, UVWASI_FDFLAG_APPEND));
#endif /* _WIN32 */
  CHECK(uvwasi_fd_fdstat_set_rights(&uvw, 100, 0, 0));
  CHECK(uvwasi_fd_filestat_get(&uvw, 100, &test_filestat));
  CHECK(uvwasi_fd_filestat_set_size(&uvw, 100, 0));
  CHECK(uvwasi_fd_filestat_set_times(&uvw, 99, 0, 0, UVWASI_FILESTAT_SET_ATIM));
  CHECK(uvwasi_fd_pread(&uvw, 100, &test_iovec, 2, 10, &test_size));
  CHECK(uvwasi_fd_prestat_get(&uvw, 100, &test_prestat));
  CHECK(uvwasi_fd_prestat_dir_name(&uvw, 100, test_str, 10));
  CHECK(uvwasi_fd_pwrite(&uvw, 100, &test_ciovec, 2, 10, &test_size));
  CHECK(uvwasi_fd_read(&uvw, 100, &test_iovec, 2, &test_size));
#if !defined(_WIN32) && !defined(__ANDROID__)
  CHECK(uvwasi_fd_readdir(&uvw, 100, test_void, 3, test_dircookie, &test_size));
#else
  assert(UVWASI_ENOSYS ==
      uvwasi_fd_readdir(&uvw, 100, test_void, 3, test_dircookie, &test_size));
#endif /* !defined(_WIN32) && !defined(__ANDROID__) */
  CHECK(uvwasi_fd_renumber(&uvw, 100, 2));
  CHECK(uvwasi_fd_seek(&uvw, 100, 10, UVWASI_WHENCE_CUR, &test_filesize));
  CHECK(uvwasi_fd_sync(&uvw, 100));
  CHECK(uvwasi_fd_tell(&uvw, 100, &test_filesize));
  CHECK(uvwasi_fd_write(&uvw, 100, &test_ciovec, 2, &test_size));
  CHECK(uvwasi_path_create_directory(&uvw, 100, "x", 2));
  CHECK(uvwasi_path_filestat_get(&uvw, 100, 0, test_str, 5, &test_filestat));
  CHECK(uvwasi_path_filestat_set_times(&uvw, 100, 0, test_str, 4, 5, 6, 7));
  CHECK(uvwasi_path_link(&uvw, 100, 4, test_str, 5, 100, test_str, 6));
  CHECK(uvwasi_path_open(&uvw, 100, 0, test_str, 4, 5, 6, 7, 8, &test_fd));
  CHECK(uvwasi_path_readlink(&uvw, 100, test_str, 4, test_str, 5, &test_size));
  CHECK(uvwasi_path_remove_directory(&uvw, 99, "x", 2));
  CHECK(uvwasi_path_rename(&uvw, 99, test_str, 4, 99, test_str, 5));
  CHECK(uvwasi_path_symlink(&uvw, test_str, 5, 99, test_str, 5));
  CHECK(uvwasi_path_unlink_file(&uvw, 100, test_str, 10));

  uvwasi_destroy(&uvw);
  return 0;
}
