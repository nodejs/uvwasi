#include <assert.h>
#include <stdio.h>
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

  printf("uvwasi version: %s (%d)\n",
         UVWASI_VERSION_STRING,
         UVWASI_VERSION_HEX);

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
  char buf[env_buf_size > argv_buf_size ? env_buf_size : argv_buf_size];

  char** args_get_argv = calloc(argc, sizeof(char*));
  r = uvwasi_args_get(uvw, args_get_argv, buf);
  assert(r == 0);
  assert(strcmp(args_get_argv[0], init_options.argv[0]) == 0);
  assert(strcmp(args_get_argv[1], init_options.argv[1]) == 0);
  assert(strcmp(args_get_argv[2], init_options.argv[2]) == 0);


  char** env_get_env = calloc(envc, sizeof(char*));
  r = uvwasi_environ_get(uvw, env_get_env, buf);
  assert(r == 0);
  for (i = 0; i < envc; ++i)
    assert(strlen(env_get_env[i]) > 0);

  uvwasi_fd_t dirfd = 3;
  uvwasi_lookupflags_t dirflags = 1;
  const char* path = "../uvwasi/./foo.txt";
  uvwasi_oflags_t o_flags = UVWASI_O_CREAT;
  uvwasi_rights_t fs_rights_base = UVWASI_RIGHT_FD_DATASYNC |
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
  assert(fd >= 0);

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

  r = uvwasi_fd_fdstat_get(uvw, fd, &fdstat_buf);
  assert(r == 0);
  assert(fdstat_buf.fs_filetype == UVWASI_FILETYPE_REGULAR_FILE);
  assert(fdstat_buf.fs_rights_inheriting == 0);
  assert(fdstat_buf.fs_flags == 0);
  printf("uvwasi_fd_fdstat_get()\n");
  printf("\tstats.fs_rights_base = %llu\n", fdstat_buf.fs_rights_base);

  r = uvwasi_fd_fdstat_get(uvw, fd, &fdstat_buf);
  assert(r == 0);
  assert(fdstat_buf.fs_filetype == UVWASI_FILETYPE_REGULAR_FILE);
  assert(fdstat_buf.fs_rights_inheriting == 0);
  assert(fdstat_buf.fs_flags == 0);
  printf("uvwasi_fd_fdstat_get()\n");
  printf("\tstats.fs_rights_base = %llu\n", fdstat_buf.fs_rights_base);

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

  uvwasi_prestat_t prestat;
  r = uvwasi_fd_prestat_get(uvw, dirfd, &prestat);
  assert(r == 0);
  assert(prestat.pr_type == UVWASI_PREOPENTYPE_DIR);
  assert(prestat.u.dir.pr_name_len ==
         strlen(init_options.preopens[0].mapped_path) + 1);

  char prestat_buf[prestat.u.dir.pr_name_len + 1];
  r = uvwasi_fd_prestat_dir_name(uvw, dirfd, prestat_buf, sizeof(prestat_buf));
  assert(r == 0);
  assert(strcmp(prestat_buf, init_options.preopens[0].mapped_path) == 0);

  r = uvwasi_path_create_directory(uvw,
                                   dirfd,
                                   "test_dir",
                                   strlen("test_dir") + 1);
  assert(r == 0);

  r = uvwasi_path_create_directory(uvw,
                                   dirfd,
                                   "../test_dir",
                                   strlen("../test_dir") + 1);
  assert(r == UVWASI_ENOTCAPABLE);

  r = uvwasi_path_remove_directory(uvw,
                                   dirfd,
                                   "test_dir",
                                   strlen("test_dir") + 1);
  assert(r == 0);

  uvwasi_destroy(uvw);  /* Clean up memory. */
  uvwasi_proc_exit(uvw, 75);  /* proc_exit() actually still works. */

  return 0;
}
