#ifndef __UVWASI_FD_TABLE_H__
#define __UVWASI_FD_TABLE_H__

#include <stdint.h>
#include "uv.h"
#include "wasi_types.h"

/* TODO(cjihrig): libuv needs a PATH_MAX_BYTES. */
#ifdef _WIN32
/* MAX_PATH is in characters, not bytes. Make sure we have enough headroom. */
# define PATH_MAX_BYTES (MAX_PATH * 4)
#else
# include <limits.h>
# define PATH_MAX_BYTES (PATH_MAX)
#endif


struct uvwasi_fd_preopen_t {
  char real_path[PATH_MAX_BYTES];
};

struct uvwasi_fd_wrap_t {
  uvwasi_fd_t id;
  uv_file fd;
  char path[PATH_MAX_BYTES];
  uvwasi_filetype_t type; /* TODO(cjihrig): It probably isn't safe to cache. */
  uvwasi_rights_t rights_base;
  uvwasi_rights_t rights_inheriting;
  struct uvwasi_fd_preopen_t* preopen;
  int valid;
};

struct uvwasi_fd_table_t {
  struct uvwasi_fd_wrap_t* fds;
  uint32_t size;
  uint32_t used;
};

uvwasi_errno_t uvwasi_fd_table_init(struct uvwasi_fd_table_t* table,
                                    uint32_t init_size);
uvwasi_errno_t uvwasi_fd_table_insert_preopen(struct uvwasi_fd_table_t* table,
                                              const uv_file fd,
                                              const char* path,
                                              const char* real_path);
uvwasi_errno_t uvwasi_fd_table_insert_fd(struct uvwasi_fd_table_t* table,
                                         const uv_file fd,
                                         int flags,
                                         const char* path,
                                         uvwasi_rights_t rights_base,
                                         uvwasi_rights_t rights_inheriting,
                                         struct uvwasi_fd_wrap_t* wrap);
uvwasi_errno_t uvwasi_fd_table_get(struct uvwasi_fd_table_t* table,
                                   const uvwasi_fd_t id,
                                   struct uvwasi_fd_wrap_t** wrap,
                                   uvwasi_rights_t rights_base,
                                   uvwasi_rights_t rights_inheriting);
uvwasi_errno_t uvwasi_fd_table_remove(struct uvwasi_fd_table_t* table,
                                      const uvwasi_fd_t id);

#endif /* __UVWASI_FD_TABLE_H__ */
