#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "uv.h"
#include "uvwasi.h"

#ifdef __APPLE__
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#elif !defined(_MSC_VER)
extern char** environ;
#endif


int main(void) {
  uvwasi_options_t init_options;
  uvwasi_t uvwasi;
  uvwasi_t* uvw;
  uvwasi_fdstat_t fdstat_buf;
  uvwasi_errno_t r;
  size_t argc;
  size_t argv_buf_size;
  size_t envc;
  size_t env_buf_size;
  size_t i;

  assert(UVWASI_VERSION_HEX == 1);
  assert(strcmp(UVWASI_VERSION_STRING, "0.0.1") == 0);
  assert(strcmp(UVWASI_VERSION_WASI, "snapshot_0") == 0);

  uvw = &uvwasi;
  init_options.fd_table_size = 3;
  init_options.argc = 3;
  init_options.argv = calloc(3, sizeof(char*));
  init_options.argv[0] = "--foo=bar";
  init_options.argv[1] = "-baz";
  init_options.argv[2] = "100";
  init_options.envp = (char**) environ;
  init_options.preopenc = 1;
  init_options.preopens = calloc(1, sizeof(uvwasi_preopen_t));
  init_options.preopens[0].mapped_path = "/var";
  init_options.preopens[0].real_path = ".";
  init_options.allocator = NULL;

  r = uvwasi_init(uvw, &init_options);
  assert(r == 0);

  r = uvwasi_args_sizes_get(uvw, &argc, &argv_buf_size);
  assert(r == 0);
  assert(argc == init_options.argc);
  assert(argv_buf_size > 0);

  r = uvwasi_environ_sizes_get(uvw, &envc, &env_buf_size);
  assert(r == 0);
  assert(envc > 0);
  assert(env_buf_size > 0);
  char* buf = malloc(env_buf_size > argv_buf_size ? env_buf_size : argv_buf_size);

  char** args_get_argv = calloc(argc, sizeof(char*));
  assert(args_get_argv != NULL);
  r = uvwasi_args_get(uvw, args_get_argv, buf);
  assert(r == 0);
  assert(strcmp(args_get_argv[0], init_options.argv[0]) == 0);
  assert(strcmp(args_get_argv[1], init_options.argv[1]) == 0);
  assert(strcmp(args_get_argv[2], init_options.argv[2]) == 0);


  char** env_get_env = calloc(envc, sizeof(char*));
  assert(env_get_env != NULL);
  r = uvwasi_environ_get(uvw, env_get_env, buf);
  assert(r == 0);
  for (i = 0; i < envc; ++i)
    assert(strlen(env_get_env[i]) > 0);
  free(buf);

  uvwasi_fd_t dirfd = 3;
  uvwasi_lookupflags_t dirflags = 1;
  const char* path = "./foo.txt";
  uvwasi_oflags_t o_flags = UVWASI_O_CREAT;
  uvwasi_rights_t fs_rights_base = UVWASI_RIGHT_FD_DATASYNC |
                                   UVWASI_RIGHT_FD_ALLOCATE |
                                   UVWASI_RIGHT_FD_FDSTAT_SET_FLAGS |
                                   UVWASI_RIGHT_FD_FILESTAT_GET |
                                   UVWASI_RIGHT_FD_FILESTAT_SET_SIZE |
                                   UVWASI_RIGHT_FD_ADVISE |
                                   UVWASI_RIGHT_FD_READ |
                                   UVWASI_RIGHT_FD_SYNC |
                                   UVWASI_RIGHT_PATH_READLINK |
                                   UVWASI_RIGHT_PATH_UNLINK_FILE;
  uvwasi_rights_t fs_rights_inheriting = 1;
  uvwasi_fdflags_t fs_flags = 1;
  uvwasi_fd_t fd;

  r = uvwasi_path_open(uvw,
                       dirfd,
                       dirflags,
                       path,
                       strlen(path) + 1,
                       o_flags,
                       fs_rights_base,
                       fs_rights_inheriting,
                       fs_flags,
                       &fd);
  assert(r == 0);

  r = uvwasi_fd_sync(uvw, fd);
  assert(r == 0);

  r = uvwasi_fd_filestat_set_size(uvw, fd, 106);
  assert(r == 0);

  r = uvwasi_fd_datasync(uvw, fd);
  assert(r == 0);

  r = uvwasi_fd_advise(uvw, fd, 0, 0, UVWASI_ADVICE_DONTNEED);
  assert(r == 0);

  uvwasi_filestat_t stats;
  r = uvwasi_fd_filestat_get(uvw, fd, &stats);
  assert(r == 0);
  assert(stats.st_dev > 0);
  assert(stats.st_ino > 0);
  assert(stats.st_nlink == 1);
  assert(stats.st_size == 106);
  assert(stats.st_filetype == UVWASI_FILETYPE_REGULAR_FILE);
  assert(stats.st_atim > 0);
  assert(stats.st_mtim > 0);
  assert(stats.st_ctim > 0);

  r = uvwasi_fd_allocate(uvw, fd, 9, 141);
  assert(r == 0);

  r = uvwasi_fd_filestat_get(uvw, fd, &stats);
  assert(r == 0);
  assert(stats.st_size == 150);

  r = uvwasi_fd_fdstat_get(uvw, fd, &fdstat_buf);
  assert(r == 0);
  assert(fdstat_buf.fs_filetype == UVWASI_FILETYPE_REGULAR_FILE);
  assert(fdstat_buf.fs_rights_inheriting == 0);
  uvwasi_rights_t old_rights_base = fdstat_buf.fs_rights_base;
  uvwasi_fdflags_t old_flags = fdstat_buf.fs_flags;

  r = uvwasi_fd_fdstat_set_flags(uvw, fd, UVWASI_FDFLAG_NONBLOCK);
  assert(r == 0);

  r = uvwasi_fd_fdstat_get(uvw, fd, &fdstat_buf);
  assert(r == 0);
  assert(fdstat_buf.fs_filetype == UVWASI_FILETYPE_REGULAR_FILE);
  assert(fdstat_buf.fs_rights_inheriting == 0);
  assert(fdstat_buf.fs_rights_base == old_rights_base);
#ifdef _WIN32
  assert(fdstat_buf.fs_flags == 0);
#else
  assert(fdstat_buf.fs_flags != old_flags);
#endif /* _WIN32 */

  uvwasi_iovec_t* iovs;
  size_t iovs_len;
  size_t nread;

  nread = 0;
  iovs_len = 2;
  iovs = calloc(iovs_len, sizeof(*iovs));
  for (i = 0; i < iovs_len; ++i) {
    iovs[i].buf_len = 10;
    iovs[i].buf = malloc(1024);
  }

  r = uvwasi_fd_read(uvw, fd, iovs, iovs_len, &nread);
  assert(r == 0);
  assert(nread == 20);
  free(iovs);

  r = uvwasi_fd_fdstat_set_rights(uvw, fd, UVWASI_RIGHT_FD_FILESTAT_GET, 0);
  assert(r == 0);

  r = uvwasi_fd_close(uvw, fd);
  assert(r == 0);

  r = uvwasi_path_unlink_file(uvw, dirfd, path, strlen(path) + 1);
  assert(r == 0);

  char readdir_buf[2048];
  size_t bufused;
  uvwasi_dircookie_t cookie;
  cookie = UVWASI_DIRCOOKIE_START;
  r = uvwasi_fd_readdir(uvw,
                        dirfd,
                        readdir_buf,
                        sizeof(readdir_buf),
                        cookie,
                        &bufused);
  assert(r == 0);

  uvwasi_destroy(uvw);  /* Clean up memory. */
  uvwasi_proc_exit(uvw, 75);  /* proc_exit() actually still works. */

  return 0;
}
