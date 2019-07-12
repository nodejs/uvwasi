#include <stdlib.h>

#include "uvwasi.h"
#include "uv.h"
#include "fd_table.h"

#define NANOS_PER_SEC 1000000000


uvwasi_errno_t uvwasi_init(uvwasi_t* uvwasi, int preopenc, char**preopen_dirs) {
  /* TODO(cjihrig): Support mapping preopened directories to other names. */
  uv_fs_t realpath_req;
  uv_fs_t open_req;
  int flags;
  int i;
  uvwasi_errno_t r;

  /* TODO(cjihrig): Make the initial size configurable. */
  r = uvwasi_fd_table_init(&uvwasi->fds, 1024);

  flags = UV_FS_O_CREAT;

  for (i = 0; i < preopenc; ++i) {
    r = uv_fs_realpath(NULL, &realpath_req, preopen_dirs[i], NULL);
    /* TODO(cjihrig): Handle errors. */
    r = uv_fs_open(NULL, &open_req, realpath_req.ptr, flags, 0666, NULL);
    /* TODO(cjihrig): Handle errors. */
    r = uvwasi_fd_table_insert_preopen(&uvwasi->fds,
                                       open_req.result,
                                       realpath_req.ptr,
                                       realpath_req.ptr);
    /* TODO(cjihrig): Handle errors. */
    uv_fs_req_cleanup(&realpath_req);
    uv_fs_req_cleanup(&open_req);
  }

  return UVWASI_ESUCCESS;
}


static uvwasi_timestamp_t uvwasi__timespec_to_timestamp(const uv_timespec_t* ts
                                                       ) {
  /* TODO(cjihrig): Handle overflow. */
  return (uvwasi_timestamp_t) ts->tv_sec * NANOS_PER_SEC + ts->tv_nsec;
}


static uvwasi_errno_t uvwasi__translate_uv_error(int err) {
  switch (err) {
    case UV_E2BIG:           return UVWASI_E2BIG;
    case UV_EACCES:          return UVWASI_EACCES;
    case UV_EADDRINUSE:      return UVWASI_EADDRINUSE;
    case UV_EADDRNOTAVAIL:   return UVWASI_EADDRNOTAVAIL;
    case UV_EAFNOSUPPORT:    return UVWASI_EAFNOSUPPORT;
    case UV_EAGAIN:          return UVWASI_EAGAIN;
    case UV_EALREADY:        return UVWASI_EALREADY;
    case UV_EBADF:           return UVWASI_EBADF;
    case UV_EBUSY:           return UVWASI_EBUSY;
    case UV_ECANCELED:       return UVWASI_ECANCELED;
    case UV_ECONNABORTED:    return UVWASI_ECONNABORTED;
    case UV_ECONNREFUSED:    return UVWASI_ECONNREFUSED;
    case UV_ECONNRESET:      return UVWASI_ECONNRESET;
    case UV_EDESTADDRREQ:    return UVWASI_EDESTADDRREQ;
    case UV_EEXIST:          return UVWASI_EEXIST;
    case UV_EFAULT:          return UVWASI_EFAULT;
    case UV_EFBIG:           return UVWASI_EFBIG;
    case UV_EHOSTUNREACH:    return UVWASI_EHOSTUNREACH;
    case UV_EINTR:           return UVWASI_EINTR;
    case UV_EINVAL:          return UVWASI_EINVAL;
    case UV_EIO:             return UVWASI_EIO;
    case UV_EISCONN:         return UVWASI_EISCONN;
    case UV_EISDIR:          return UVWASI_EISDIR;
    case UV_ELOOP:           return UVWASI_ELOOP;
    case UV_EMFILE:          return UVWASI_EMFILE;
    case UV_EMLINK:          return UVWASI_EMLINK;
    case UV_EMSGSIZE:        return UVWASI_EMSGSIZE;
    case UV_ENAMETOOLONG:    return UVWASI_ENAMETOOLONG;
    case UV_ENETDOWN:        return UVWASI_ENETDOWN;
    case UV_ENETUNREACH:     return UVWASI_ENETUNREACH;
    case UV_ENFILE:          return UVWASI_ENFILE;
    case UV_ENOBUFS:         return UVWASI_ENOBUFS;
    case UV_ENODEV:          return UVWASI_ENODEV;
    case UV_ENOENT:          return UVWASI_ENOENT;
    case UV_ENOMEM:          return UVWASI_ENOMEM;
    case UV_ENOPROTOOPT:     return UVWASI_ENOPROTOOPT;
    case UV_ENOSPC:          return UVWASI_ENOSPC;
    case UV_ENOSYS:          return UVWASI_ENOSYS;
    case UV_ENOTCONN:        return UVWASI_ENOTCONN;
    case UV_ENOTDIR:         return UVWASI_ENOTDIR;
    case UV_ENOTEMPTY:       return UVWASI_ENOTEMPTY;
    case UV_ENOTSOCK:        return UVWASI_ENOTSOCK;
    case UV_ENOTSUP:         return UVWASI_ENOTSUP;
    case UV_ENXIO:           return UVWASI_ENXIO;
    case UV_EPERM:           return UVWASI_EPERM;
    case UV_EPIPE:           return UVWASI_EPIPE;
    case UV_EPROTO:          return UVWASI_EPROTO;
    case UV_EPROTONOSUPPORT: return UVWASI_EPROTONOSUPPORT;
    case UV_EPROTOTYPE:      return UVWASI_EPROTOTYPE;
    case UV_ERANGE:          return UVWASI_ERANGE;
    case UV_EROFS:           return UVWASI_EROFS;
    case UV_ESPIPE:          return UVWASI_ESPIPE;
    case UV_ESRCH:           return UVWASI_ESRCH;
    case UV_ETIMEDOUT:       return UVWASI_ETIMEDOUT;
    case UV_ETXTBSY:         return UVWASI_ETXTBSY;
    case UV_EXDEV:           return UVWASI_EXDEV;
    case 0:                  return UVWASI_ESUCCESS;
    /* TODO(cjihrig): libuv errors with no corresponding WASI error.
    case UV_EAI_ADDRFAMILY:
    case UV_EAI_AGAIN:
    case UV_EAI_BADFLAGS:
    case UV_EAI_BADHINTS:
    case UV_EAI_CANCELED:
    case UV_EAI_FAIL:
    case UV_EAI_FAMILY:
    case UV_EAI_MEMORY:
    case UV_EAI_NODATA:
    case UV_EAI_NONAME:
    case UV_EAI_OVERFLOW:
    case UV_EAI_PROTOCOL:
    case UV_EAI_SERVICE:
    case UV_EAI_SOCKTYPE:
    case UV_ECHARSET:
    case UV_ENONET:
    case UV_EOF:
    case UV_ESHUTDOWN:
    case UV_UNKNOWN:
    */
    default:
      /* libuv errors are < 0 */
      if (err > 0)
        return err;

      return UVWASI_ENOSYS;
  }
}


uvwasi_errno_t uvwasi_args_get(uvwasi_t* uvwasi, char** argv, char* argv_buf) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_args_sizes_get(uvwasi_t* uvwasi,
                                     size_t* argc,
                                     size_t* argv_buf_size) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_clock_res_get(uvwasi_t* uvwasi,
                                    uvwasi_clockid_t clock_id,
                                    uvwasi_timestamp_t* resolution) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_clock_time_get(uvwasi_t* uvwasi,
                                     uvwasi_clockid_t clock_id,
                                     uvwasi_timestamp_t precision,
                                     uvwasi_timestamp_t* time) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_environ_get(uvwasi_t* uvwasi,
                                  char** environ,
                                  char* environ_buf) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_environ_sizes_get(uvwasi_t* uvwasi,
                                        size_t* environ_count,
                                        size_t* environ_buf_size) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_fd_advise(uvwasi_t* uvwasi,
                                uvwasi_fd_t fd,
                                uvwasi_filesize_t offset,
                                uvwasi_filesize_t len,
                                uvwasi_advice_t advice) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_fd_allocate(uvwasi_t* uvwasi,
                                  uvwasi_fd_t fd,
                                  uvwasi_filesize_t offset,
                                  uvwasi_filesize_t len) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_fd_close(uvwasi_t* uvwasi, uvwasi_fd_t fd) {
  struct uvwasi_fd_wrap_t wrap;
  uvwasi_errno_t err;
  uv_fs_t req;
  int r;

  err = uvwasi_fd_table_get(&uvwasi->fds, fd, &wrap, 0, 0);
  if (err != UVWASI_ESUCCESS)
    return err;

  r = uv_fs_close(NULL, &req, wrap.fd, NULL);
  uv_fs_req_cleanup(&req);

  if (r != 0)
    return uvwasi__translate_uv_error(r);

  err = uvwasi_fd_table_remove(&uvwasi->fds, fd);
  if (err != UVWASI_ESUCCESS)
    return err;

  return UVWASI_ESUCCESS;
}


uvwasi_errno_t uvwasi_fd_datasync(uvwasi_t* uvwasi, uvwasi_fd_t fd) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_fd_fdstat_get(uvwasi_t* uvwasi,
                                    uvwasi_fd_t fd,
                                    uvwasi_fdstat_t* buf) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_fd_fdstat_set_flags(uvwasi_t* uvwasi,
                                          uvwasi_fd_t fd,
                                          uvwasi_fdflags_t flags) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_fd_fdstat_set_rights(uvwasi_t* uvwasi,
                                           uvwasi_fd_t fd,
                                           uvwasi_rights_t fs_rights_base,
                                           uvwasi_rights_t fs_rights_inheriting
                                          ) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_fd_filestat_get(uvwasi_t* uvwasi,
                                      uvwasi_fd_t fd,
                                      uvwasi_filestat_t* buf) {
  struct uvwasi_fd_wrap_t wrap;
  uv_fs_t req;
  uvwasi_errno_t err;
  int r;

  err = uvwasi_fd_table_get(&uvwasi->fds,
                            fd,
                            &wrap,
                            UVWASI_RIGHT_FD_FILESTAT_GET,
                            0);
  if (err != UVWASI_ESUCCESS)
    return err;

  r = uv_fs_fstat(NULL, &req, wrap.fd, NULL);
  if (r != 0) {
    uv_fs_req_cleanup(&req);
    return uvwasi__translate_uv_error(r);
  }

  buf->st_dev = req.statbuf.st_dev;
  buf->st_ino = req.statbuf.st_ino;
  buf->st_nlink = req.statbuf.st_nlink;
  buf->st_size = req.statbuf.st_size;
  buf->st_filetype = wrap.type;
  buf->st_atim = uvwasi__timespec_to_timestamp(&req.statbuf.st_atim);
  buf->st_mtim = uvwasi__timespec_to_timestamp(&req.statbuf.st_mtim);
  buf->st_ctim = uvwasi__timespec_to_timestamp(&req.statbuf.st_ctim);
  uv_fs_req_cleanup(&req);

  return UVWASI_ESUCCESS;
}


uvwasi_errno_t uvwasi_fd_filestat_set_size(uvwasi_t* uvwasi,
                                           uvwasi_fd_t fd,
                                           uvwasi_filesize_t st_size) {
  /* TODO(cjihrig): uv_fs_ftruncate() takes an int64_t. st_size is uint64_t. */
  struct uvwasi_fd_wrap_t wrap;
  uv_fs_t req;
  uvwasi_errno_t err;
  int r;

  err = uvwasi_fd_table_get(&uvwasi->fds,
                            fd,
                            &wrap,
                            UVWASI_RIGHT_FD_FILESTAT_SET_SIZE,
                            0);
  if (err != UVWASI_ESUCCESS)
    return err;

  r = uv_fs_ftruncate(NULL, &req, wrap.fd, st_size, NULL);
  uv_fs_req_cleanup(&req);

  if (r != 0)
    return uvwasi__translate_uv_error(r);

  return UVWASI_ESUCCESS;
}


uvwasi_errno_t uvwasi_fd_filestat_set_times(uvwasi_t* uvwasi,
                                            uvwasi_fd_t fd,
                                            uvwasi_timestamp_t st_atim,
                                            uvwasi_timestamp_t st_mtim,
                                            uvwasi_fstflags_t fst_flags) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_fd_pread(uvwasi_t* uvwasi,
                               uvwasi_fd_t fd,
                               const uvwasi_iovec_t* iovs,
                               size_t iovs_len,
                               uvwasi_filesize_t offset,
                               size_t* nread) {
  return UVWASI_ENOTSUP;
}


/* TODO(cjihrig): uvwasi_prestat_t is not defined. */
uvwasi_errno_t uvwasi_fd_prestat_get(uvwasi_t* uvwasi,
                                     uvwasi_fd_t fd
                                     /* , uvwasi_prestat_t* buf */) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_fd_prestat_dir_name(uvwasi_t* uvwasi,
                                          uvwasi_fd_t fd,
                                          char* path,
                                          size_t path_len) {
  struct uvwasi_fd_wrap_t wrap;
  uvwasi_errno_t err;
  int size;

  if (uvwasi == NULL || path == NULL)
    return UVWASI_EINVAL;

  err = uvwasi_fd_table_get(&uvwasi->fds, fd, &wrap, 0, 0);
  if (err != UVWASI_ESUCCESS)
    return err;
  if (wrap.preopen == NULL)
    return UVWASI_EBADF;

  size = strlen(wrap.path) + 1;
  if (size > path_len)
    return UVWASI_ENOBUFS;

  memcpy(path, wrap.path, size);
  return UVWASI_ESUCCESS;
}


uvwasi_errno_t uvwasi_fd_pwrite(uvwasi_t* uvwasi,
                                uvwasi_fd_t fd,
                                const uvwasi_ciovec_t* iovs,
                                size_t iovs_len,
                                uvwasi_filesize_t offset,
                                size_t* nwritten) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_fd_read(uvwasi_t* uvwasi,
                              uvwasi_fd_t fd,
                              const uvwasi_iovec_t* iovs,
                              size_t iovs_len,
                              size_t* nread) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_fd_readdir(uvwasi_t* uvwasi,
                                 uvwasi_fd_t fd,
                                 void* buf,
                                 size_t buf_len,
                                 uvwasi_dircookie_t cookie,
                                 size_t* bufused) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_fd_renumber(uvwasi_t* uvwasi,
                                  uvwasi_fd_t from,
                                  uvwasi_fd_t to) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_fd_seek(uvwasi_t* uvwasi,
                              uvwasi_fd_t fd,
                              uvwasi_filedelta_t offset,
                              uvwasi_whence_t whence,
                              uvwasi_filesize_t* newoffset) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_fd_sync(uvwasi_t* uvwasi, uvwasi_fd_t fd) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_fd_tell(uvwasi_t* uvwasi,
                              uvwasi_fd_t fd,
                              uvwasi_filesize_t* offset) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_fd_write(uvwasi_t* uvwasi,
                               uvwasi_fd_t fd,
                               const uvwasi_ciovec_t* iovs,
                               size_t iovs_len,
                               size_t* nwritten) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_path_create_directory(uvwasi_t* uvwasi,
                                            uvwasi_fd_t fd,
                                            const char* path,
                                            size_t path_len) {
  /* TODO(cjihrig): path_len is currently unused. */
  struct uvwasi_fd_wrap_t wrap;
  uv_fs_t req;
  uvwasi_errno_t err;
  int r;

  err = uvwasi_fd_table_get(&uvwasi->fds,
                            fd,
                            &wrap,
                            UVWASI_RIGHT_PATH_CREATE_DIRECTORY,
                            0);
  if (err != UVWASI_ESUCCESS)
    return err;

  /* TODO(cjihrig): Resolve path from fd. */
  r = uv_fs_mkdir(NULL, &req, path, 0777, NULL);
  uv_fs_req_cleanup(&req);

  if (r != 0)
    return uvwasi__translate_uv_error(r);

  return UVWASI_ESUCCESS;
}


uvwasi_errno_t uvwasi_path_filestat_get(uvwasi_t* uvwasi,
                                        uvwasi_fd_t fd,
                                        uvwasi_lookupflags_t flags,
                                        const char* path,
                                        size_t path_len,
                                        uvwasi_filestat_t* buf) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_path_filestat_set_times(uvwasi_t* uvwasi,
                                              uvwasi_fd_t fd,
                                              uvwasi_lookupflags_t flags,
                                              const char* path,
                                              size_t path_len,
                                              uvwasi_timestamp_t st_atim,
                                              uvwasi_timestamp_t st_mtim,
                                              uvwasi_fstflags_t fst_flags) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_path_link(uvwasi_t* uvwasi,
                                uvwasi_fd_t old_fd,
                                uvwasi_lookupflags_t old_flags,
                                const char* old_path,
                                size_t old_path_len,
                                uvwasi_fd_t new_fd,
                                const char* new_path,
                                size_t new_path_len) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_path_open(uvwasi_t* uvwasi,
                                uvwasi_fd_t dirfd,
                                uvwasi_lookupflags_t dirflags,
                                const char* path,
                                size_t path_len,
                                uvwasi_oflags_t o_flags,
                                uvwasi_rights_t fs_rights_base,
                                uvwasi_rights_t fs_rights_inheriting,
                                uvwasi_fdflags_t fs_flags,
                                uvwasi_fd_t* fd) {
  /* TODO(cjihrig): Check dirfd? */
  uvwasi_rights_t needed_inheriting;
  uvwasi_rights_t needed_base;
  struct uvwasi_fd_wrap_t wrap;
  uv_fs_t req;
  int flags;
  int read;
  int write;
  int r;

  read = 0 != (fs_rights_base & (UVWASI_RIGHT_FD_READ |
                                 UVWASI_RIGHT_FD_READDIR));
  write = 0 != (fs_rights_base & (UVWASI_RIGHT_FD_DATASYNC |
                                  UVWASI_RIGHT_FD_WRITE |
                                  UVWASI_RIGHT_FD_ALLOCATE |
                                  UVWASI_RIGHT_FD_FILESTAT_SET_SIZE));
  flags = write ? read ? UV_FS_O_RDWR : UV_FS_O_WRONLY : UV_FS_O_RDONLY;
  needed_base = UVWASI_RIGHT_PATH_OPEN;
  needed_inheriting = fs_rights_base | fs_rights_inheriting;

  if ((o_flags & UVWASI_O_CREAT) != 0) {
    flags |= UV_FS_O_CREAT;
    needed_base |= UVWASI_RIGHT_PATH_CREATE_FILE;
  }
  if ((o_flags & UVWASI_O_DIRECTORY) != 0)
    flags |= UV_FS_O_DIRECTORY;
  if ((o_flags & UVWASI_O_EXCL) != 0)
    flags |= UV_FS_O_EXCL;
  if ((o_flags & UVWASI_O_TRUNC) != 0) {
    flags |= UV_FS_O_TRUNC;
    needed_base |= UVWASI_RIGHT_PATH_FILESTAT_SET_SIZE;
  }

  if ((fs_flags & UVWASI_FDFLAG_APPEND) != 0)
    flags |= UV_FS_O_APPEND;
  if ((fs_flags & UVWASI_FDFLAG_DSYNC) != 0) {
    flags |= UV_FS_O_DSYNC;
    needed_inheriting |= UVWASI_RIGHT_FD_DATASYNC;
  }
  if ((fs_flags & UVWASI_FDFLAG_NONBLOCK) != 0)
    flags |= UV_FS_O_NONBLOCK;
  if ((fs_flags & UVWASI_FDFLAG_RSYNC) != 0) {
#ifdef O_RSYNC
    flags |= O_RSYNC; /* libuv has no UV_FS_O_RSYNC. */
#else
    flags |= UV_FS_O_SYNC;
#endif
    needed_inheriting |= UVWASI_RIGHT_FD_SYNC;
  }
  if ((fs_flags & UVWASI_FDFLAG_SYNC) != 0) {
    flags |= UV_FS_O_SYNC;
    needed_inheriting |= UVWASI_RIGHT_FD_SYNC;
  }
  if (write && (flags & (UV_FS_O_APPEND | UV_FS_O_TRUNC)) == 0)
    needed_inheriting |= UVWASI_RIGHT_FD_SEEK;

  /* TODO(cjihrig): Resolve path to dirfd's path and check for '..' */

  r = uv_fs_open(NULL, &req, path, flags, 0666, NULL);
  uv_fs_req_cleanup(&req);

  if (r < 0)
    return uvwasi__translate_uv_error(r);

  r = uvwasi_fd_table_insert_fd(&uvwasi->fds,
                                r,
                                flags,
                                path,
                                fs_rights_base,
                                fs_rights_inheriting,
                                &wrap);
  /* TODO(cjihrig): Handle errors. Also need to close the new fd on error. */

  *fd = wrap.id;
  return UVWASI_ESUCCESS;
}


uvwasi_errno_t uvwasi_path_readlink(uvwasi_t* uvwasi,
                                    uvwasi_fd_t fd,
                                    const char* path,
                                    size_t path_len,
                                    char* buf,
                                    size_t buf_len,
                                    size_t* bufused) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_path_remove_directory(uvwasi_t* uvwasi,
                                            uvwasi_fd_t fd,
                                            const char* path,
                                            size_t path_len) {
  /* TODO(cjihrig): path_len is currently unused. */
  struct uvwasi_fd_wrap_t wrap;
  uv_fs_t req;
  uvwasi_errno_t err;
  int r;

  err = uvwasi_fd_table_get(&uvwasi->fds,
                            fd,
                            &wrap,
                            UVWASI_RIGHT_PATH_REMOVE_DIRECTORY,
                            0);
  if (err != UVWASI_ESUCCESS)
    return err;

  /* TODO(cjihrig): Resolve path from fd. */
  r = uv_fs_rmdir(NULL, &req, path, NULL);
  uv_fs_req_cleanup(&req);

  if (r != 0)
    return uvwasi__translate_uv_error(r);

  return UVWASI_ESUCCESS;
}


uvwasi_errno_t uvwasi_path_rename(uvwasi_t* uvwasi,
                                  uvwasi_fd_t old_fd,
                                  const char* old_path,
                                  size_t old_path_len,
                                  uvwasi_fd_t new_fd,
                                  const char* new_path,
                                  size_t new_path_len) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_path_symlink(uvwasi_t* uvwasi,
                                   const char* old_path,
                                   size_t old_path_len,
                                   uvwasi_fd_t fd,
                                   const char* new_path,
                                   size_t new_path_len) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_path_unlink_file(uvwasi_t* uvwasi,
                                       uvwasi_fd_t fd,
                                       const char* path,
                                       size_t path_len) {
  struct uvwasi_fd_wrap_t wrap;
  uv_fs_t req;
  uvwasi_errno_t err;
  int r;

  err = uvwasi_fd_table_get(&uvwasi->fds,
                            fd,
                            &wrap,
                            UVWASI_RIGHT_PATH_UNLINK_FILE,
                            0);
  if (err != UVWASI_ESUCCESS)
    return err;

  /* TODO(cjihrig): Resolve path from fd. */
  r = uv_fs_unlink(NULL, &req, path, NULL);
  uv_fs_req_cleanup(&req);

  if (r != 0)
    return uvwasi__translate_uv_error(r);

  return UVWASI_ESUCCESS;
}


uvwasi_errno_t uvwasi_poll_oneoff(uvwasi_t* uvwasi,
                                  const uvwasi_subscription_t* in,
                                  uvwasi_event_t* out,
                                  size_t nsubscriptions,
                                  size_t* nevents) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_proc_exit(uvwasi_t* uvwasi, uvwasi_exitcode_t rval) {
  exit(rval);
  return UVWASI_ESUCCESS; /* This doesn't happen. */
}


uvwasi_errno_t uvwasi_proc_raise(uvwasi_t* uvwasi, uvwasi_signal_t sig) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_random_get(uvwasi_t* uvwasi, void* buf, size_t buf_len) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_sched_yield(uvwasi_t* uvwasi) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_sock_recv(uvwasi_t* uvwasi,
                                uvwasi_fd_t sock,
                                const uvwasi_iovec_t* ri_data,
                                size_t ri_data_len,
                                uvwasi_riflags_t ri_flags,
                                size_t* ro_datalen,
                                uvwasi_roflags_t* ro_flags) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_sock_send(uvwasi_t* uvwasi,
                                uvwasi_fd_t sock,
                                const uvwasi_ciovec_t* si_data,
                                size_t si_data_len,
                                uvwasi_siflags_t si_flags,
                                size_t* so_datalen) {
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_sock_shutdown(uvwasi_t* uvwasi,
                                    uvwasi_fd_t sock,
                                    uvwasi_sdflags_t how) {
  return UVWASI_ENOTSUP;
}
