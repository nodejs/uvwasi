#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "uvwasi.h"

#define CHECK(expr) assert(UVWASI_EINVAL == (expr))

int main(void) {
  uvwasi_t uvw;
  char* test_str = "foo";
  void* test_void;
  uvwasi_size_t test_size = 5;
  uvwasi_clockid_t test_clock = UVWASI_CLOCK_MONOTONIC;
  uvwasi_timestamp_t test_ts = 1000;
  uvwasi_fdstat_t test_fdstat;
  uvwasi_fdflags_t test_fdflags = UVWASI_FDFLAG_APPEND;
  uvwasi_filestat_t test_filestat;
  uvwasi_iovec_t test_iovec;
  uvwasi_ciovec_t test_ciovec;
  uvwasi_prestat_t test_prestat;
  uvwasi_dircookie_t test_dircookie = 0;
  uvwasi_filesize_t test_filesize;
  uvwasi_subscription_t test_sub;
  uvwasi_event_t test_event;
  uvwasi_fd_t test_fd;

  test_void = (void*) &test_fdstat;

  CHECK(uvwasi_args_get(NULL, &test_str, test_str));
  CHECK(uvwasi_args_get(&uvw, NULL, test_str));
  CHECK(uvwasi_args_get(&uvw, &test_str, NULL));

  CHECK(uvwasi_args_sizes_get(NULL, &test_size, &test_size));
  CHECK(uvwasi_args_sizes_get(&uvw, NULL, &test_size));
  CHECK(uvwasi_args_sizes_get(&uvw, &test_size, NULL));

  CHECK(uvwasi_clock_res_get(NULL, test_clock, &test_ts));
  CHECK(uvwasi_clock_res_get(&uvw, test_clock, NULL));

  CHECK(uvwasi_clock_time_get(NULL, test_clock, test_ts, &test_ts));
  CHECK(uvwasi_clock_time_get(&uvw, test_clock, test_ts, NULL));

  CHECK(uvwasi_environ_get(NULL, &test_str, test_str));
  CHECK(uvwasi_environ_get(&uvw, NULL, test_str));
  CHECK(uvwasi_environ_get(&uvw, &test_str, NULL));

  CHECK(uvwasi_environ_sizes_get(NULL, &test_size, &test_size));
  CHECK(uvwasi_environ_sizes_get(&uvw, NULL, &test_size));
  CHECK(uvwasi_environ_sizes_get(&uvw, &test_size, NULL));

  CHECK(uvwasi_fd_advise(NULL, 3, 10, 20, UVWASI_ADVICE_DONTNEED));

  CHECK(uvwasi_fd_allocate(NULL, 3, 10, 20));

  CHECK(uvwasi_fd_close(NULL, 3));

  CHECK(uvwasi_fd_datasync(NULL, 3));

  CHECK(uvwasi_fd_fdstat_get(NULL, 3, &test_fdstat));
  CHECK(uvwasi_fd_fdstat_get(&uvw, 3, NULL));

#ifndef _WIN32
  CHECK(uvwasi_fd_fdstat_set_flags(NULL, 3, test_fdflags));
#else
  assert(UVWASI_ENOSYS == uvwasi_fd_fdstat_set_flags(NULL, 3, test_fdflags));
#endif /* _WIN32 */

  CHECK(uvwasi_fd_fdstat_set_rights(NULL, 3, 0, 0));

  CHECK(uvwasi_fd_filestat_get(NULL, 3, &test_filestat));
  CHECK(uvwasi_fd_filestat_get(&uvw, 3, NULL));

  CHECK(uvwasi_fd_filestat_set_size(NULL, 3, 0));

  CHECK(uvwasi_fd_filestat_set_times(NULL, 3, 0, 0, UVWASI_FILESTAT_SET_ATIM));

  CHECK(uvwasi_fd_pread(NULL, 3, &test_iovec, 2, 10, &test_size));
  CHECK(uvwasi_fd_pread(&uvw, 3, NULL, 2, 10, &test_size));
  CHECK(uvwasi_fd_pread(&uvw, 3, &test_iovec, 2, 10, NULL));

  CHECK(uvwasi_fd_prestat_get(NULL, 3, &test_prestat));
  CHECK(uvwasi_fd_prestat_get(&uvw, 3, NULL));

  CHECK(uvwasi_fd_prestat_dir_name(NULL, 3, test_str, 10));
  CHECK(uvwasi_fd_prestat_dir_name(&uvw, 3, NULL, 10));

  CHECK(uvwasi_fd_pwrite(NULL, 3, &test_ciovec, 2, 10, &test_size));
  CHECK(uvwasi_fd_pwrite(&uvw, 3, NULL, 2, 10, &test_size));
  CHECK(uvwasi_fd_pwrite(&uvw, 3, &test_ciovec, 2, 10, NULL));

  CHECK(uvwasi_fd_read(NULL, 3, &test_iovec, 2, &test_size));
  CHECK(uvwasi_fd_read(&uvw, 3, NULL, 2, &test_size));
  CHECK(uvwasi_fd_read(&uvw, 3, &test_iovec, 2, NULL));

  CHECK(uvwasi_fd_readdir(NULL, 3, test_void, 3, test_dircookie, &test_size));
  CHECK(uvwasi_fd_readdir(&uvw, 3, NULL, 3, test_dircookie, &test_size));
  CHECK(uvwasi_fd_readdir(&uvw, 3, test_void, 3, test_dircookie, NULL));

  CHECK(uvwasi_fd_renumber(NULL, 3, 2));

  CHECK(uvwasi_fd_seek(NULL, 3, 10, UVWASI_WHENCE_CUR, &test_filesize));
  CHECK(uvwasi_fd_seek(&uvw, 3, 10, UVWASI_WHENCE_CUR, NULL));

  CHECK(uvwasi_fd_sync(NULL, 3));

  CHECK(uvwasi_fd_tell(NULL, 3, &test_filesize));
  CHECK(uvwasi_fd_tell(&uvw, 3, NULL));

  CHECK(uvwasi_fd_write(NULL, 3, &test_ciovec, 2, &test_size));
  CHECK(uvwasi_fd_write(&uvw, 3, NULL, 2, &test_size));
  CHECK(uvwasi_fd_write(&uvw, 3, &test_ciovec, 2, NULL));

  CHECK(uvwasi_path_create_directory(NULL, 3, "x", 2));
  CHECK(uvwasi_path_create_directory(&uvw, 3, NULL, 2));

  CHECK(uvwasi_path_filestat_get(NULL, 3, 0, test_str, 5, &test_filestat));
  CHECK(uvwasi_path_filestat_get(&uvw, 3, 0, NULL, 5, &test_filestat));
  CHECK(uvwasi_path_filestat_get(&uvw, 3, 0, test_str, 5, NULL));

  CHECK(uvwasi_path_filestat_set_times(NULL, 3, 0, test_str, 4, 5, 6, 7));
  CHECK(uvwasi_path_filestat_set_times(&uvw, 3, 0, NULL, 4, 5, 6, 7));

  CHECK(uvwasi_path_link(NULL, 3, 4, test_str, 5, 3, test_str, 6));
  CHECK(uvwasi_path_link(&uvw, 3, 4, NULL, 5, 3, test_str, 6));
  CHECK(uvwasi_path_link(&uvw, 3, 4, test_str, 5, 3, NULL, 6));

  CHECK(uvwasi_path_open(NULL, 3, 0, test_str, 4, 5, 6, 7, 8, &test_fd));
  CHECK(uvwasi_path_open(&uvw, 3, 0, NULL, 4, 5, 6, 7, 8, &test_fd));
  CHECK(uvwasi_path_open(&uvw, 3, 0, test_str, 4, 5, 6, 7, 8, NULL));

  CHECK(uvwasi_path_readlink(NULL, 3, test_str, 4, test_str, 5, &test_size));
  CHECK(uvwasi_path_readlink(&uvw, 3, NULL, 4, test_str, 5, &test_size));
  CHECK(uvwasi_path_readlink(&uvw, 3, test_str, 4, NULL, 5, &test_size));
  CHECK(uvwasi_path_readlink(&uvw, 3, test_str, 4, test_str, 5, NULL));

  CHECK(uvwasi_path_remove_directory(NULL, 3, "x", 2));
  CHECK(uvwasi_path_remove_directory(&uvw, 3, NULL, 2));

  CHECK(uvwasi_path_rename(NULL, 3, test_str, 4, 3, test_str, 5));
  CHECK(uvwasi_path_rename(&uvw, 3, NULL, 4, 3, test_str, 5));
  CHECK(uvwasi_path_rename(&uvw, 3, test_str, 4, 3, NULL, 5));

  CHECK(uvwasi_path_symlink(NULL, test_str, 5, 3, test_str, 5));
  CHECK(uvwasi_path_symlink(&uvw, NULL, 5, 3, test_str, 5));
  CHECK(uvwasi_path_symlink(&uvw, test_str, 5, 3, NULL, 5));

  CHECK(uvwasi_path_unlink_file(NULL, 3, test_str, 10));
  CHECK(uvwasi_path_unlink_file(&uvw, 3, NULL, 10));

  CHECK(uvwasi_poll_oneoff(NULL, &test_sub, &test_event, 5, &test_size));
  CHECK(uvwasi_poll_oneoff(&uvw, NULL, &test_event, 5, &test_size));
  CHECK(uvwasi_poll_oneoff(&uvw, &test_sub, NULL, 5, &test_size));
  CHECK(uvwasi_poll_oneoff(&uvw, &test_sub, &test_event, 0, &test_size));
  CHECK(uvwasi_poll_oneoff(&uvw, &test_sub, &test_event, 5, NULL));

  CHECK(uvwasi_random_get(NULL, test_void, 10));
  CHECK(uvwasi_random_get(&uvw, NULL, 10));

  CHECK(uvwasi_sched_yield(NULL));

  return 0;
}
