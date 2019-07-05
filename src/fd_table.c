#include <sys/stat.h>
#include "uv.h"
#include "fd_table.h"
#include "wasi_types.h"


#define UVWASI__RIGHTS_ALL (UVWASI_RIGHT_FD_DATASYNC |                        \
                            UVWASI_RIGHT_FD_READ |                            \
                            UVWASI_RIGHT_FD_SEEK |                            \
                            UVWASI_RIGHT_FD_FDSTAT_SET_FLAGS |                \
                            UVWASI_RIGHT_FD_SYNC |                            \
                            UVWASI_RIGHT_FD_TELL |                            \
                            UVWASI_RIGHT_FD_WRITE |                           \
                            UVWASI_RIGHT_FD_ADVISE |                          \
                            UVWASI_RIGHT_FD_ALLOCATE |                        \
                            UVWASI_RIGHT_PATH_CREATE_DIRECTORY |              \
                            UVWASI_RIGHT_PATH_CREATE_FILE |                   \
                            UVWASI_RIGHT_PATH_LINK_SOURCE |                   \
                            UVWASI_RIGHT_PATH_LINK_TARGET |                   \
                            UVWASI_RIGHT_PATH_OPEN |                          \
                            UVWASI_RIGHT_FD_READDIR |                         \
                            UVWASI_RIGHT_PATH_READLINK |                      \
                            UVWASI_RIGHT_PATH_RENAME_SOURCE |                 \
                            UVWASI_RIGHT_PATH_RENAME_TARGET |                 \
                            UVWASI_RIGHT_PATH_FILESTAT_GET |                  \
                            UVWASI_RIGHT_PATH_FILESTAT_SET_SIZE |             \
                            UVWASI_RIGHT_PATH_FILESTAT_SET_TIMES |            \
                            UVWASI_RIGHT_FD_FILESTAT_GET |                    \
                            UVWASI_RIGHT_FD_FILESTAT_SET_TIMES |              \
                            UVWASI_RIGHT_FD_FILESTAT_SET_SIZE |               \
                            UVWASI_RIGHT_PATH_SYMLINK |                       \
                            UVWASI_RIGHT_PATH_UNLINK_FILE |                   \
                            UVWASI_RIGHT_PATH_REMOVE_DIRECTORY |              \
                            UVWASI_RIGHT_POLL_FD_READWRITE |                  \
                            UVWASI_RIGHT_SOCK_SHUTDOWN)

#define UVWASI__RIGHTS_BLOCK_DEVICE_BASE UVWASI__RIGHTS_ALL
#define UVWASI__RIGHTS_BLOCK_DEVICE_INHERITING UVWASI__RIGHTS_ALL

#define UVWASI__RIGHTS_CHARACTER_DEVICE_BASE UVWASI__RIGHTS_ALL
#define UVWASI__RIGHTS_CHARACTER_DEVICE_INHERITING UVWASI__RIGHTS_ALL

#define UVWASI__RIGHTS_REGULAR_FILE_BASE (UVWASI_RIGHT_FD_DATASYNC |          \
                                          UVWASI_RIGHT_FD_READ |              \
                                          UVWASI_RIGHT_FD_SEEK |              \
                                          UVWASI_RIGHT_FD_FDSTAT_SET_FLAGS |  \
                                          UVWASI_RIGHT_FD_SYNC |              \
                                          UVWASI_RIGHT_FD_TELL |              \
                                          UVWASI_RIGHT_FD_WRITE |             \
                                          UVWASI_RIGHT_FD_ADVISE |            \
                                          UVWASI_RIGHT_FD_ALLOCATE |          \
                                          UVWASI_RIGHT_FD_FILESTAT_GET |      \
                                          UVWASI_RIGHT_FD_FILESTAT_SET_SIZE | \
                                          UVWASI_RIGHT_FD_FILESTAT_SET_TIMES |\
                                          UVWASI_RIGHT_POLL_FD_READWRITE)
#define UVWASI__RIGHTS_REGULAR_FILE_INHERITING 0

#define UVWASI__RIGHTS_DIRECTORY_BASE (UVWASI_RIGHT_FD_FDSTAT_SET_FLAGS |     \
                                       UVWASI_RIGHT_FD_SYNC |                 \
                                       UVWASI_RIGHT_FD_ADVISE |               \
                                       UVWASI_RIGHT_PATH_CREATE_DIRECTORY |   \
                                       UVWASI_RIGHT_PATH_CREATE_FILE |        \
                                       UVWASI_RIGHT_PATH_LINK_SOURCE |        \
                                       UVWASI_RIGHT_PATH_LINK_TARGET |        \
                                       UVWASI_RIGHT_PATH_OPEN |               \
                                       UVWASI_RIGHT_FD_READDIR |              \
                                       UVWASI_RIGHT_PATH_READLINK |           \
                                       UVWASI_RIGHT_PATH_RENAME_SOURCE |      \
                                       UVWASI_RIGHT_PATH_RENAME_TARGET |      \
                                       UVWASI_RIGHT_PATH_FILESTAT_GET |       \
                                       UVWASI_RIGHT_PATH_FILESTAT_SET_SIZE |  \
                                       UVWASI_RIGHT_PATH_FILESTAT_SET_TIMES | \
                                       UVWASI_RIGHT_FD_FILESTAT_GET |         \
                                       UVWASI_RIGHT_FD_FILESTAT_SET_TIMES |   \
                                       UVWASI_RIGHT_PATH_SYMLINK |            \
                                       UVWASI_RIGHT_PATH_UNLINK_FILE |        \
                                       UVWASI_RIGHT_PATH_REMOVE_DIRECTORY |   \
                                       UVWASI_RIGHT_POLL_FD_READWRITE)
#define UVWASI__RIGHTS_DIRECTORY_INHERITING (UVWASI__RIGHTS_DIRECTORY_BASE |  \
                                             UVWASI__RIGHTS_REGULAR_FILE_BASE)

#define UVWASI__RIGHTS_SOCKET_BASE (UVWASI_RIGHT_FD_READ |                    \
                                    UVWASI_RIGHT_FD_FDSTAT_SET_FLAGS |        \
                                    UVWASI_RIGHT_FD_WRITE |                   \
                                    UVWASI_RIGHT_FD_FILESTAT_GET |            \
                                    UVWASI_RIGHT_POLL_FD_READWRITE |          \
                                    UVWASI_RIGHT_SOCK_SHUTDOWN)
#define UVWASI__RIGHTS_SOCKET_INHERITING UVWASI__RIGHTS_ALL;

#define UVWASI__RIGHTS_TTY_BASE (UVWASI_RIGHT_FD_READ |                       \
                                 UVWASI_RIGHT_FD_FDSTAT_SET_FLAGS |           \
                                 UVWASI_RIGHT_FD_WRITE |                      \
                                 UVWASI_RIGHT_FD_FILESTAT_GET |               \
                                 UVWASI_RIGHT_POLL_FD_READWRITE)
#define UVWASI__RIGHTS_TTY_INHERITING 0

static uvwasi_errno_t uvwasi__get_type_and_rights(uv_file fd,
                                          int flags,
                                          uvwasi_filetype_t* type,
                                          uvwasi_rights_t* rights_base,
                                          uvwasi_rights_t* rights_inheriting) {
  uv_fs_t req;
  uint64_t mode;
  uv_handle_type handle_type;
  int read_or_write_only;
  int r;

  r = uv_fs_fstat(NULL, &req, fd, NULL);
  /* TODO(cjihrig): Handle errors. */
  mode = req.statbuf.st_mode;
  uv_fs_req_cleanup(&req);

  if (S_ISREG(mode)) {
    *type = UVWASI_FILETYPE_REGULAR_FILE;
    *rights_base = UVWASI__RIGHTS_REGULAR_FILE_BASE;
    *rights_inheriting = UVWASI__RIGHTS_REGULAR_FILE_INHERITING;
  } else if (S_ISDIR(mode)) {
    *type = UVWASI_FILETYPE_DIRECTORY;
    *rights_base = UVWASI__RIGHTS_DIRECTORY_BASE;
    *rights_inheriting = UVWASI__RIGHTS_DIRECTORY_INHERITING;
  } else if (S_ISSOCK(mode)) {
    handle_type = uv_guess_handle(fd);

    if (handle_type == UV_TCP) {
      *type = UVWASI_FILETYPE_SOCKET_STREAM;
    } else if (handle_type == UV_UDP) {
      *type = UVWASI_FILETYPE_SOCKET_DGRAM;
    } else {
      *type = UVWASI_FILETYPE_UNKNOWN;
      *rights_base = 0;
      *rights_inheriting = 0;
      return UVWASI_EINVAL;
    }

    *rights_base = UVWASI__RIGHTS_SOCKET_BASE;
    *rights_inheriting = UVWASI__RIGHTS_SOCKET_INHERITING;
  } else if (S_ISFIFO(mode)) {
    *type = UVWASI_FILETYPE_SOCKET_STREAM;
    *rights_base = UVWASI__RIGHTS_SOCKET_BASE;
    *rights_inheriting = UVWASI__RIGHTS_SOCKET_INHERITING;
  } else if (S_ISBLK(mode)) {
    *type = UVWASI_FILETYPE_BLOCK_DEVICE;
    *rights_base = UVWASI__RIGHTS_BLOCK_DEVICE_BASE;
    *rights_inheriting = UVWASI__RIGHTS_BLOCK_DEVICE_INHERITING;
  } else if (S_ISCHR(mode)) {
    *type = UVWASI_FILETYPE_CHARACTER_DEVICE;

    if (uv_guess_handle(fd) == UV_TTY) {
      *rights_base = UVWASI__RIGHTS_TTY_BASE;
      *rights_inheriting = UVWASI__RIGHTS_TTY_INHERITING;
    } else {
      *rights_base = UVWASI__RIGHTS_CHARACTER_DEVICE_BASE;
      *rights_inheriting = UVWASI__RIGHTS_CHARACTER_DEVICE_INHERITING;
    }
  } else {
    *type = UVWASI_FILETYPE_UNKNOWN;
    *rights_base = 0;
    *rights_inheriting = 0;
    return UVWASI_EINVAL;
  }

  /* Disable read/write bits depending on access mode. */
  read_or_write_only = flags & (UV_FS_O_RDONLY | UV_FS_O_WRONLY | UV_FS_O_RDWR);

  if (read_or_write_only == UV_FS_O_RDONLY)
    *rights_base &= ~UVWASI_RIGHT_FD_WRITE;
  else if (read_or_write_only == UV_FS_O_WRONLY)
    *rights_base &= ~UVWASI_RIGHT_FD_READ;

  return UVWASI_ESUCCESS;
}


uvwasi_errno_t uvwasi_fd_table_init(struct uvwasi_fd_table_t* table) {
  table->size = 1024; /* TODO(cjihrig): Make this zero. */

  return UVWASI_ESUCCESS;
}


uvwasi_errno_t uvwasi_fd_table_insert_fd(struct uvwasi_fd_table_t* table,
                                         const uv_file fd,
                                         int flags,
                                         const char* path,
                                         uvwasi_rights_t rights_base,
                                         uvwasi_rights_t rights_inheriting,
                                         struct uvwasi_fd_wrap_t* wrap) {
  struct uvwasi_fd_wrap_t* entry;
  uvwasi_filetype_t type;
  uvwasi_rights_t max_base;
  uvwasi_rights_t max_inheriting;
  uvwasi_errno_t r;

  r = uvwasi__get_type_and_rights(fd, flags, &type, &max_base, &max_inheriting);
  /* TODO(cjihrig): Handle errors. */

  entry = &table->fds[0];
  entry->id = 100; /* TODO(cjihrig): Set this to 3. 0-2 are stdio. */
  entry->fd = fd;

  if (path != NULL)
    strcpy(entry->path, path); /* TODO(cjihrig): Use realpath here? */

  entry->type = type;
  entry->rights_base = rights_base & max_base;
  entry->rights_inheriting = rights_inheriting & max_inheriting;
  *wrap = *entry;

  return UVWASI_ESUCCESS;
}

uvwasi_errno_t uvwasi_fd_table_get(struct uvwasi_fd_table_t* table,
                                   const uvwasi_fd_t id,
                                   struct uvwasi_fd_wrap_t* wrap,
                                   uvwasi_rights_t rights_base,
                                   uvwasi_rights_t rights_inheriting) {
  struct uvwasi_fd_wrap_t* entry;

  /* TODO(cjihrig): Implement this for real. */
  entry = &table->fds[0];

  if (entry->id != id)
    return UVWASI_EBADF;

  /* Validate that the fd has the necessary rights. */
  if ((~entry->rights_base & rights_base) != 0 ||
      (~entry->rights_inheriting & rights_inheriting) != 0)
    return UVWASI_ENOTCAPABLE;

  *wrap = *entry;
  return UVWASI_ESUCCESS;
}
