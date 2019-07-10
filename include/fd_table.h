#ifndef __UVWASI_FD_TABLE_H__
#define __UVWASI_FD_TABLE_H__

#include <stdint.h>
#include "uv.h"
#include "wasi_types.h"


struct uvwasi_fd_wrap_t {
  uvwasi_fd_t id;
  uv_file fd;
  char path[1024];  /* TODO(cjihrig): Make this dynamic. */
  uvwasi_filetype_t type; /* TODO(cjihrig): It probably isn't safe to cache. */
  uvwasi_rights_t rights_base;
  uvwasi_rights_t rights_inheriting;
};

struct uvwasi_fd_table_t {
  struct uvwasi_fd_wrap_t fds[1024]; /* TODO(cjihrig): Support resizing, etc. */
  uint32_t size;
};

uvwasi_errno_t uvwasi_fd_table_init(struct uvwasi_fd_table_t* table);
uvwasi_errno_t uvwasi_fd_table_insert_fd(struct uvwasi_fd_table_t* table,
                                         const uv_file fd,
                                         int flags,
                                         const char* path,
                                         uvwasi_rights_t rights_base,
                                         uvwasi_rights_t rights_inheriting,
                                         struct uvwasi_fd_wrap_t* wrap);
uvwasi_errno_t uvwasi_fd_table_get(struct uvwasi_fd_table_t* table,
                                   const uvwasi_fd_t id,
                                   struct uvwasi_fd_wrap_t* wrap,
                                   uvwasi_rights_t rights_base,
                                   uvwasi_rights_t rights_inheriting);

#endif /* __UVWASI_FD_TABLE_H__ */
