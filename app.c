#include <stdio.h>
#include <string.h>
#include "uv.h"
#include "uvwasi.h"


int main(void) {
  char* preopen_dirs[] = { "." };
  uvwasi_t uvwasi;
  uvwasi_t* uvw;
  uvwasi_errno_t r;

  uvw = &uvwasi;
  r = uvwasi_init(uvw, 1, preopen_dirs);
  printf("uvwasi_init() r = %d\n", r);

  uvwasi_fd_t dirfd = 1;
  uvwasi_lookupflags_t dirflags = 1;
  const char* path = "./foo.txt";
  uvwasi_oflags_t o_flags = UVWASI_O_CREAT;
  uvwasi_rights_t fs_rights_base = UVWASI_RIGHT_FD_FILESTAT_GET |
                                   UVWASI_RIGHT_FD_FILESTAT_SET_SIZE |
                                   UVWASI_RIGHT_FD_READ;
  uvwasi_rights_t fs_rights_inheriting = 1;
  uvwasi_fdflags_t fs_flags = 1;
  uvwasi_fd_t fd;

  r = uvwasi_path_open(uvw,
                       dirfd,
                       dirflags,
                       path,
                       strlen(path),
                       o_flags,
                       fs_rights_base,
                       fs_rights_inheriting,
                       fs_flags,
                       &fd);
  printf("open r = %d, fd = %d\n", r, fd);

  r = uvwasi_fd_filestat_set_size(uvw, fd, 106);
  printf("set_size r = %d\n", r);

  uvwasi_filestat_t stats;
  r = uvwasi_fd_filestat_get(uvw, fd, &stats);
  printf("fstat r = %d\n", r);
  printf("\tstats.st_dev = %llu\n", stats.st_dev);
  printf("\tstats.st_ino = %llu\n", stats.st_ino);
  printf("\tstats.st_nlink = %u\n", stats.st_nlink);
  printf("\tstats.st_size = %llu\n", stats.st_size);
  printf("\tstats.st_filetype = %hhu\n", stats.st_filetype);
  printf("\tstats.st_atim = %llu\n", stats.st_atim);
  printf("\tstats.st_mtim = %llu\n", stats.st_mtim);
  printf("\tstats.st_ctim = %llu\n", stats.st_ctim);

  r = uvwasi_fd_close(uvw, fd);
  printf("close r = %d\n", r);

  r = uvwasi_path_unlink_file(uvw, dirfd, path, strlen(path));
  printf("unlink_file r = %d\n", r);

  return 0;
}
