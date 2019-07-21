#include <stdlib.h>

#ifndef _WIN32
# include <sched.h>
# include <sys/types.h>
# include <unistd.h>
#else
# include <processthreadsapi.h>
#endif /* _WIN32 */

#include "uvwasi.h"
#include "uv.h"
#include "fd_table.h"

#define NANOS_PER_SEC 1000000000


uvwasi_errno_t uvwasi_init(uvwasi_t* uvwasi, uvwasi_options_t* options) {
  uv_fs_t realpath_req;
  uv_fs_t open_req;
  uvwasi_errno_t err;
  size_t args_size;
  size_t size;
  size_t offset;
  size_t env_count;
  size_t env_buf_size;
  int flags;
  int i;
  int r;

  if (uvwasi == NULL || options == NULL || options->fd_table_size == 0)
    return UVWASI_EINVAL;

  args_size = 0;
  for (i = 0; i < options->argc; ++i) {
    args_size += strlen(options->argv[i]) + 1;
  }

  uvwasi->argc = options->argc;
  uvwasi->argv_buf_size = args_size;
  /* TODO(cjihrig): Add error handling. */
  uvwasi->argv_buf = malloc(args_size);
  uvwasi->argv = calloc(options->argc, sizeof(char*));

  offset = 0;
  for (i = 0; i < options->argc; ++i) {
    size = strlen(options->argv[i]) + 1;
    memcpy(uvwasi->argv_buf + offset, options->argv[i], size);
    uvwasi->argv[i] = uvwasi->argv_buf + offset;
    offset += size;
  }

  env_count = 0;
  env_buf_size = 0;
  while (options->envp[env_count] != NULL) {
    env_buf_size += strlen(options->envp[env_count]) + 1;
    env_count++;
  }

  uvwasi->envc = env_count;
  uvwasi->env_buf_size = env_buf_size;
  uvwasi->env_buf = malloc(env_buf_size);
  uvwasi->env = calloc(env_count, sizeof(char*));

  offset = 0;
  for (i = 0; i < env_count; ++i) {
    size = strlen(options->envp[i]) + 1;
    memcpy(uvwasi->env_buf + offset, options->envp[i], size);
    uvwasi->env[i] = uvwasi->env_buf + offset;
    offset += size;
  }

  for (i = 0; i < options->preopenc; ++i) {
    if (options->preopens[i].real_path == NULL ||
        options->preopens[i].mapped_path == NULL) {
      return UVWASI_EINVAL;
    }
  }

  err = uvwasi_fd_table_init(&uvwasi->fds, options->fd_table_size);
  if (err != UVWASI_ESUCCESS)
    return err;

  flags = UV_FS_O_CREAT;

  for (i = 0; i < options->preopenc; ++i) {
    r = uv_fs_realpath(NULL,
                       &realpath_req,
                       options->preopens[i].real_path,
                       NULL);
    /* TODO(cjihrig): Handle errors. */
    r = uv_fs_open(NULL, &open_req, realpath_req.ptr, flags, 0666, NULL);
    /* TODO(cjihrig): Handle errors. */
    err = uvwasi_fd_table_insert_preopen(&uvwasi->fds,
                                         open_req.result,
                                         options->preopens[i].mapped_path,
                                         realpath_req.ptr);
    /* TODO(cjihrig): Handle errors. */
    uv_fs_req_cleanup(&realpath_req);
    uv_fs_req_cleanup(&open_req);
  }

  return UVWASI_ESUCCESS;
}


static uvwasi_errno_t uvwasi__resolve_path(const struct uvwasi_fd_wrap_t* fd,
                                           const char* path,
                                           size_t path_len,
                                           char* resolved_path) {
  /* TODO(cjihrig): Actually resolve path to fd. */
  if (path_len > PATH_MAX_BYTES)
    return UVWASI_ENOBUFS;

  memcpy(resolved_path, path, path_len);
  resolved_path[path_len] = '\0';
  return UVWASI_ESUCCESS;
}


static uvwasi_timestamp_t uvwasi__timespec_to_timestamp(const uv_timespec_t* ts
                                                       ) {
  /* TODO(cjihrig): Handle overflow. */
  return (uvwasi_timestamp_t) ts->tv_sec * NANOS_PER_SEC + ts->tv_nsec;
}


static int uvwasi__translate_to_uv_signal(uvwasi_signal_t sig) {
  switch (sig) {
#ifdef SIGABRT
    case UVWASI_SIGABRT:   return SIGABRT;
#endif
#ifdef SIGALRM
    case UVWASI_SIGALRM:   return SIGALRM;
#endif
#ifdef SIGBUS
    case UVWASI_SIGBUS:    return SIGBUS;
#endif
#ifdef SIGCHLD
    case UVWASI_SIGCHLD:   return SIGCHLD;
#endif
#ifdef SIGCONT
    case UVWASI_SIGCONT:   return SIGCONT;
#endif
#ifdef SIGFPE
    case UVWASI_SIGFPE:    return SIGFPE;
#endif
#ifdef SIGHUP
    case UVWASI_SIGHUP:    return SIGHUP;
#endif
#ifdef SIGILL
    case UVWASI_SIGILL:    return SIGILL;
#endif
#ifdef SIGINT
    case UVWASI_SIGINT:    return SIGINT;
#endif
#ifdef SIGKILL
    case UVWASI_SIGKILL:   return SIGKILL;
#endif
#ifdef SIGPIPE
    case UVWASI_SIGPIPE:   return SIGPIPE;
#endif
#ifdef SIGQUIT
    case UVWASI_SIGQUIT:   return SIGQUIT;
#endif
#ifdef SIGSEGV
    case UVWASI_SIGSEGV:   return SIGSEGV;
#endif
#ifdef SIGSTOP
    case UVWASI_SIGSTOP:   return SIGSTOP;
#endif
#ifdef SIGSYS
    case UVWASI_SIGSYS:    return SIGSYS;
#endif
#ifdef SIGTERM
    case UVWASI_SIGTERM:   return SIGTERM;
#endif
#ifdef SIGTRAP
    case UVWASI_SIGTRAP:   return SIGTRAP;
#endif
#ifdef SIGTSTP
    case UVWASI_SIGTSTP:   return SIGTSTP;
#endif
#ifdef SIGTTIN
    case UVWASI_SIGTTIN:   return SIGTTIN;
#endif
#ifdef SIGTTOU
    case UVWASI_SIGTTOU:   return SIGTTOU;
#endif
#ifdef SIGURG
    case UVWASI_SIGURG:    return SIGURG;
#endif
#ifdef SIGUSR1
    case UVWASI_SIGUSR1:   return SIGUSR1;
#endif
#ifdef SIGUSR2
    case UVWASI_SIGUSR2:   return SIGUSR2;
#endif
#ifdef SIGVTALRM
    case UVWASI_SIGVTALRM: return SIGVTALRM;
#endif
#ifdef SIGXCPU
    case UVWASI_SIGXCPU:   return SIGXCPU;
#endif
#ifdef SIGXFSZ
    case UVWASI_SIGXFSZ:   return SIGXFSZ;
#endif
    default:               return -1;
  }
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
    /* The following libuv error codes have no corresponding WASI error code:
          UV_EAI_ADDRFAMILY, UV_EAI_AGAIN, UV_EAI_BADFLAGS, UV_EAI_BADHINTS,
          UV_EAI_CANCELED, UV_EAI_FAIL, UV_EAI_FAMILY, UV_EAI_MEMORY,
          UV_EAI_NODATA, UV_EAI_NONAME, UV_EAI_OVERFLOW, UV_EAI_PROTOCOL,
          UV_EAI_SERVICE, UV_EAI_SOCKTYPE, UV_ECHARSET, UV_ENONET, UV_EOF,
          UV_ESHUTDOWN, UV_UNKNOWN
    */
    default:
      /* libuv errors are < 0 */
      if (err > 0)
        return err;

      return UVWASI_ENOSYS;
  }
}


static uvwasi_errno_t uvwasi__lseek(uv_file fd,
                                    uvwasi_filedelta_t offset,
                                    uvwasi_whence_t whence,
                                    uvwasi_filesize_t* newoffset) {
  int real_whence;

  if (whence == UVWASI_WHENCE_CUR)
    real_whence = SEEK_CUR;
  else if (whence == UVWASI_WHENCE_END)
    real_whence = SEEK_END;
  else if (whence == UVWASI_WHENCE_SET)
    real_whence = SEEK_SET;
  else
    return UVWASI_EINVAL;

#ifdef _WIN32
  int64_t r;

  r = _lseeki64(fd, offset, real_whence);
  if (-1L == r)
    return uvwasi__translate_uv_error(uv_translate_sys_error(errno));
#else
  off_t r;

  r = lseek(fd, offset, real_whence);
  if ((off_t) -1 == r)
    return uvwasi__translate_uv_error(uv_translate_sys_error(errno));
#endif /* _WIN32 */

  *newoffset = r;
  return UVWASI_ESUCCESS;
}


static uvwasi_errno_t uvwasi__setup_iovs(uv_buf_t** buffers,
                                         const uvwasi_iovec_t* iovs,
                                         size_t iovs_len) {
  uv_buf_t* bufs;
  int i;

  bufs = malloc(iovs_len * sizeof(*bufs));
  if (bufs == NULL)
    return UVWASI_ENOMEM;

  for (i = 0; i < iovs_len; ++i)
    bufs[i] = uv_buf_init(iovs[i].buf, iovs[i].buf_len);

  *buffers = bufs;
  return UVWASI_ESUCCESS;
}


static uvwasi_errno_t uvwasi__setup_ciovs(uv_buf_t** buffers,
                                         const uvwasi_ciovec_t* iovs,
                                         size_t iovs_len) {
  uv_buf_t* bufs;
  int i;

  bufs = malloc(iovs_len * sizeof(*bufs));
  if (bufs == NULL)
    return UVWASI_ENOMEM;

  for (i = 0; i < iovs_len; ++i)
    bufs[i] = uv_buf_init((char*)iovs[i].buf, iovs[i].buf_len);

  *buffers = bufs;
  return UVWASI_ESUCCESS;
}


uvwasi_errno_t uvwasi_args_get(uvwasi_t* uvwasi, char** argv, char* argv_buf) {
  int i;

  if (uvwasi == NULL || argv == NULL || argv_buf == NULL)
    return UVWASI_EINVAL;

  for (i = 0; i < uvwasi->argc; ++i) {
    argv[i] = argv_buf + (uvwasi->argv[i] - uvwasi->argv_buf);
  }

  memcpy(argv_buf, uvwasi->argv_buf, uvwasi->argv_buf_size);
  return UVWASI_ESUCCESS;
}


uvwasi_errno_t uvwasi_args_sizes_get(uvwasi_t* uvwasi,
                                     size_t* argc,
                                     size_t* argv_buf_size) {
  if (uvwasi == NULL || argc == NULL || argv_buf_size == NULL)
    return UVWASI_EINVAL;

  *argc = uvwasi->argc;
  *argv_buf_size = uvwasi->argv_buf_size;
  return UVWASI_ESUCCESS;
}


uvwasi_errno_t uvwasi_clock_res_get(uvwasi_t* uvwasi,
                                    uvwasi_clockid_t clock_id,
                                    uvwasi_timestamp_t* resolution) {
  if (uvwasi == NULL || resolution == NULL)
    return UVWASI_EINVAL;

  /*
  if (clock_id == UVWASI_CLOCK_MONOTONIC) {

  } else if (clock_id == UVWASI_CLOCK_PROCESS_CPUTIME_ID) {

  } else if (clock_id == UVWASI_CLOCK_REALTIME) {

  } else if (clock_id == UVWASI_CLOCK_THREAD_CPUTIME_ID) {

  } else {
    return UVWASI_EINVAL;
  }
  */

  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_clock_time_get(uvwasi_t* uvwasi,
                                     uvwasi_clockid_t clock_id,
                                     uvwasi_timestamp_t precision,
                                     uvwasi_timestamp_t* time) {
  if (uvwasi == NULL || time == NULL)
    return UVWASI_EINVAL;

  /*
  if (clock_id == UVWASI_CLOCK_MONOTONIC) {

  } else if (clock_id == UVWASI_CLOCK_PROCESS_CPUTIME_ID) {

  } else if (clock_id == UVWASI_CLOCK_REALTIME) {

  } else if (clock_id == UVWASI_CLOCK_THREAD_CPUTIME_ID) {

  } else {
    return UVWASI_EINVAL;
  }
  */

  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_environ_get(uvwasi_t* uvwasi,
                                  char** environ,
                                  char* environ_buf) {
  int i;

  if (uvwasi == NULL || environ == NULL || environ_buf == NULL)
    return UVWASI_EINVAL;

  for (i = 0; i < uvwasi->envc; ++i) {
    environ[i] = environ_buf + (uvwasi->env[i] - uvwasi->env_buf);
  }

  memcpy(environ_buf, uvwasi->env_buf, uvwasi->env_buf_size);
  return UVWASI_ESUCCESS;
}


uvwasi_errno_t uvwasi_environ_sizes_get(uvwasi_t* uvwasi,
                                        size_t* environ_count,
                                        size_t* environ_buf_size) {
  if (uvwasi == NULL || environ_count == NULL || environ_buf_size == NULL)
    return UVWASI_EINVAL;

  *environ_count = uvwasi->envc;
  *environ_buf_size = uvwasi->env_buf_size;
  return UVWASI_ESUCCESS;
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
  struct uvwasi_fd_wrap_t* wrap;
  uvwasi_errno_t err;
  uv_fs_t req;
  int r;

  err = uvwasi_fd_table_get(&uvwasi->fds, fd, &wrap, 0, 0);
  if (err != UVWASI_ESUCCESS)
    return err;

  r = uv_fs_close(NULL, &req, wrap->fd, NULL);
  uv_fs_req_cleanup(&req);

  if (r != 0)
    return uvwasi__translate_uv_error(r);

  err = uvwasi_fd_table_remove(&uvwasi->fds, fd);
  if (err != UVWASI_ESUCCESS)
    return err;

  return UVWASI_ESUCCESS;
}


uvwasi_errno_t uvwasi_fd_datasync(uvwasi_t* uvwasi, uvwasi_fd_t fd) {
  struct uvwasi_fd_wrap_t* wrap;
  uvwasi_errno_t err;
  uv_fs_t req;
  int r;

  err = uvwasi_fd_table_get(&uvwasi->fds,
                            fd,
                            &wrap,
                            UVWASI_RIGHT_FD_DATASYNC,
                            0);
  if (err != UVWASI_ESUCCESS)
    return err;

  r = uv_fs_fdatasync(NULL, &req, wrap->fd, NULL);
  uv_fs_req_cleanup(&req);

  if (r != 0)
    return uvwasi__translate_uv_error(r);

  return UVWASI_ESUCCESS;
}


uvwasi_errno_t uvwasi_fd_fdstat_get(uvwasi_t* uvwasi,
                                    uvwasi_fd_t fd,
                                    uvwasi_fdstat_t* buf) {
  struct uvwasi_fd_wrap_t* wrap;
  uvwasi_errno_t err;

  if (uvwasi == NULL || buf == NULL)
    return UVWASI_EINVAL;

  err = uvwasi_fd_table_get(&uvwasi->fds, fd, &wrap, 0, 0);
  if (err != UVWASI_ESUCCESS)
    return err;

  buf->fs_filetype = wrap->type;
  buf->fs_rights_base = wrap->rights_base;
  buf->fs_rights_inheriting = wrap->rights_inheriting;
  /* TODO(cjihrig): Missing support. Use F_GETFL on non-Windows. */
  buf->fs_flags = 0;

  return UVWASI_ESUCCESS;
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
  struct uvwasi_fd_wrap_t* wrap;
  uvwasi_errno_t err;

  err = uvwasi_fd_table_get(&uvwasi->fds, fd, &wrap, 0, 0);
  if (err != UVWASI_ESUCCESS)
    return err;

  /* Check for attempts to add new permissions. */
  if ((fs_rights_base | wrap->rights_base) > wrap->rights_base)
    return UVWASI_ENOTCAPABLE;
  if ((fs_rights_inheriting | wrap->rights_inheriting) >
      wrap->rights_inheriting) {
    return UVWASI_ENOTCAPABLE;
  }

  wrap->rights_base = fs_rights_base;
  wrap->rights_inheriting = fs_rights_inheriting;
  return UVWASI_ESUCCESS;
}


uvwasi_errno_t uvwasi_fd_filestat_get(uvwasi_t* uvwasi,
                                      uvwasi_fd_t fd,
                                      uvwasi_filestat_t* buf) {
  struct uvwasi_fd_wrap_t* wrap;
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

  r = uv_fs_fstat(NULL, &req, wrap->fd, NULL);
  if (r != 0) {
    uv_fs_req_cleanup(&req);
    return uvwasi__translate_uv_error(r);
  }

  buf->st_dev = req.statbuf.st_dev;
  buf->st_ino = req.statbuf.st_ino;
  buf->st_nlink = req.statbuf.st_nlink;
  buf->st_size = req.statbuf.st_size;
  buf->st_filetype = wrap->type;
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
  struct uvwasi_fd_wrap_t* wrap;
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

  r = uv_fs_ftruncate(NULL, &req, wrap->fd, st_size, NULL);
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
  struct uvwasi_fd_wrap_t* wrap;
  uv_buf_t* bufs;
  uv_fs_t req;
  uvwasi_errno_t err;
  size_t uvread;
  int r;

  if (uvwasi == NULL || iovs == NULL || nread == NULL)
    return UVWASI_EINVAL;

  err = uvwasi_fd_table_get(&uvwasi->fds,
                            fd,
                            &wrap,
                            UVWASI_RIGHT_FD_READ | UVWASI_RIGHT_FD_SEEK,
                            0);
  if (err != UVWASI_ESUCCESS)
    return err;

  err = uvwasi__setup_iovs(&bufs, iovs, iovs_len);
  if (err != UVWASI_ESUCCESS)
    return err;

  r = uv_fs_read(NULL, &req, wrap->fd, bufs, iovs_len, offset, NULL);
  uvread = req.result;
  uv_fs_req_cleanup(&req);
  free(bufs);

  if (r < 0)
    return uvwasi__translate_uv_error(r);

  *nread = uvread;
  return UVWASI_ESUCCESS;
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
  struct uvwasi_fd_wrap_t* wrap;
  uvwasi_errno_t err;
  int size;

  if (uvwasi == NULL || path == NULL)
    return UVWASI_EINVAL;

  err = uvwasi_fd_table_get(&uvwasi->fds, fd, &wrap, 0, 0);
  if (err != UVWASI_ESUCCESS)
    return err;
  if (wrap->preopen != 1)
    return UVWASI_EBADF;

  size = strlen(wrap->path) + 1;
  if (size > path_len)
    return UVWASI_ENOBUFS;

  memcpy(path, wrap->path, size);
  return UVWASI_ESUCCESS;
}


uvwasi_errno_t uvwasi_fd_pwrite(uvwasi_t* uvwasi,
                                uvwasi_fd_t fd,
                                const uvwasi_ciovec_t* iovs,
                                size_t iovs_len,
                                uvwasi_filesize_t offset,
                                size_t* nwritten) {
  struct uvwasi_fd_wrap_t* wrap;
  uv_buf_t* bufs;
  uv_fs_t req;
  uvwasi_errno_t err;
  size_t uvwritten;
  int r;

  if (uvwasi == NULL || iovs == NULL || nwritten == NULL)
    return UVWASI_EINVAL;

  err = uvwasi_fd_table_get(&uvwasi->fds,
                            fd,
                            &wrap,
                            UVWASI_RIGHT_FD_WRITE | UVWASI_RIGHT_FD_SEEK,
                            0);
  if (err != UVWASI_ESUCCESS)
    return err;

  err = uvwasi__setup_ciovs(&bufs, iovs, iovs_len);
  if (err != UVWASI_ESUCCESS)
    return err;

  r = uv_fs_write(NULL, &req, wrap->fd, bufs, iovs_len, offset, NULL);
  uvwritten = req.result;
  uv_fs_req_cleanup(&req);
  free(bufs);

  if (r < 0)
    return uvwasi__translate_uv_error(r);

  *nwritten = uvwritten;
  return UVWASI_ESUCCESS;
}


uvwasi_errno_t uvwasi_fd_read(uvwasi_t* uvwasi,
                              uvwasi_fd_t fd,
                              const uvwasi_iovec_t* iovs,
                              size_t iovs_len,
                              size_t* nread) {
  struct uvwasi_fd_wrap_t* wrap;
  uv_buf_t* bufs;
  uv_fs_t req;
  uvwasi_errno_t err;
  size_t uvread;
  int r;

  if (uvwasi == NULL || iovs == NULL || nread == NULL)
    return UVWASI_EINVAL;

  err = uvwasi_fd_table_get(&uvwasi->fds, fd, &wrap, UVWASI_RIGHT_FD_READ, 0);
  if (err != UVWASI_ESUCCESS)
    return err;

  err = uvwasi__setup_iovs(&bufs, iovs, iovs_len);
  if (err != UVWASI_ESUCCESS)
    return err;

  r = uv_fs_read(NULL, &req, wrap->fd, bufs, iovs_len, 0, NULL);
  uvread = req.result;
  uv_fs_req_cleanup(&req);
  free(bufs);

  if (r < 0)
    return uvwasi__translate_uv_error(r);

  *nread = uvread;
  return UVWASI_ESUCCESS;
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
  /* TODO(cjihrig): This function is currently untested. */
  struct uvwasi_fd_wrap_t* wrap;
  uvwasi_errno_t err;

  if (uvwasi == NULL) {
    return UVWASI_EINVAL;
  }

  err = uvwasi_fd_table_get(&uvwasi->fds, fd, &wrap, UVWASI_RIGHT_FD_SEEK, 0);
  if (err != UVWASI_ESUCCESS)
    return err;

  return uvwasi__lseek(wrap->fd, offset, whence, newoffset);
}


uvwasi_errno_t uvwasi_fd_sync(uvwasi_t* uvwasi, uvwasi_fd_t fd) {
  struct uvwasi_fd_wrap_t* wrap;
  uv_fs_t req;
  uvwasi_errno_t err;
  int r;

  err = uvwasi_fd_table_get(&uvwasi->fds,
                            fd,
                            &wrap,
                            UVWASI_RIGHT_FD_SYNC,
                            0);
  if (err != UVWASI_ESUCCESS)
    return err;

  r = uv_fs_fsync(NULL, &req, wrap->fd, NULL);
  uv_fs_req_cleanup(&req);

  if (r != 0)
    return uvwasi__translate_uv_error(r);

  return UVWASI_ESUCCESS;
}


uvwasi_errno_t uvwasi_fd_tell(uvwasi_t* uvwasi,
                              uvwasi_fd_t fd,
                              uvwasi_filesize_t* offset) {
  /* TODO(cjihrig): This function is currently untested. */
  struct uvwasi_fd_wrap_t* wrap;
  uvwasi_errno_t err;

  if (uvwasi == NULL || offset == NULL)
    return UVWASI_EINVAL;

  err = uvwasi_fd_table_get(&uvwasi->fds, fd, &wrap, UVWASI_RIGHT_FD_TELL, 0);
  if (err != UVWASI_ESUCCESS)
    return err;

  return uvwasi__lseek(wrap->fd, 0, UVWASI_WHENCE_CUR, offset);
}


uvwasi_errno_t uvwasi_fd_write(uvwasi_t* uvwasi,
                               uvwasi_fd_t fd,
                               const uvwasi_ciovec_t* iovs,
                               size_t iovs_len,
                               size_t* nwritten) {
  struct uvwasi_fd_wrap_t* wrap;
  uv_buf_t* bufs;
  uv_fs_t req;
  uvwasi_errno_t err;
  size_t uvwritten;
  int r;

  if (uvwasi == NULL || iovs == NULL || nwritten == NULL)
    return UVWASI_EINVAL;

  err = uvwasi_fd_table_get(&uvwasi->fds, fd, &wrap, UVWASI_RIGHT_FD_WRITE, 0);
  if (err != UVWASI_ESUCCESS)
    return err;

  err = uvwasi__setup_ciovs(&bufs, iovs, iovs_len);
  if (err != UVWASI_ESUCCESS)
    return err;

  r = uv_fs_write(NULL, &req, wrap->fd, bufs, iovs_len, 0, NULL);
  uvwritten = req.result;
  uv_fs_req_cleanup(&req);
  free(bufs);

  if (r < 0)
    return uvwasi__translate_uv_error(r);

  *nwritten = uvwritten;
  return UVWASI_ESUCCESS;
}


uvwasi_errno_t uvwasi_path_create_directory(uvwasi_t* uvwasi,
                                            uvwasi_fd_t fd,
                                            const char* path,
                                            size_t path_len) {
  char resolved_path[PATH_MAX_BYTES];
  struct uvwasi_fd_wrap_t* wrap;
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

  err = uvwasi__resolve_path(wrap, path, path_len, resolved_path);
  if (err != UVWASI_ESUCCESS)
    return err;

  r = uv_fs_mkdir(NULL, &req, resolved_path, 0777, NULL);
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
  /* TODO(cjihrig): flags is currently unused. */
  char resolved_path[PATH_MAX_BYTES];
  struct uvwasi_fd_wrap_t* wrap;
  uv_fs_t req;
  uvwasi_errno_t err;
  int r;

  if (uvwasi == NULL || path == NULL || buf == NULL)
    return UVWASI_EINVAL;

  err = uvwasi_fd_table_get(&uvwasi->fds,
                            fd,
                            &wrap,
                            UVWASI_RIGHT_PATH_FILESTAT_GET,
                            0);
  if (err != UVWASI_ESUCCESS)
    return err;

  err = uvwasi__resolve_path(wrap, path, path_len, resolved_path);
  if (err != UVWASI_ESUCCESS)
    return err;

  r = uv_fs_stat(NULL, &req, resolved_path, NULL);
  if (r != 0) {
    uv_fs_req_cleanup(&req);
    return uvwasi__translate_uv_error(r);
  }

  buf->st_dev = req.statbuf.st_dev;
  buf->st_ino = req.statbuf.st_ino;
  buf->st_nlink = req.statbuf.st_nlink;
  buf->st_size = req.statbuf.st_size;
  buf->st_filetype = wrap->type;
  buf->st_atim = uvwasi__timespec_to_timestamp(&req.statbuf.st_atim);
  buf->st_mtim = uvwasi__timespec_to_timestamp(&req.statbuf.st_mtim);
  buf->st_ctim = uvwasi__timespec_to_timestamp(&req.statbuf.st_ctim);
  uv_fs_req_cleanup(&req);

  return UVWASI_ESUCCESS;
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
  /* TODO(cjihrig): This function is currently untested. */
  char resolved_path[PATH_MAX_BYTES];
  struct uvwasi_fd_wrap_t* wrap;
  uvwasi_errno_t err;
  uv_fs_t req;
  size_t len;
  int r;

  err = uvwasi_fd_table_get(&uvwasi->fds,
                            fd,
                            &wrap,
                            UVWASI_RIGHT_PATH_READLINK,
                            0);
  if (err != UVWASI_ESUCCESS)
    return err;

  err = uvwasi__resolve_path(wrap, path, path_len, resolved_path);
  if (err != UVWASI_ESUCCESS)
    return err;

  r = uv_fs_readlink(NULL, &req, resolved_path, NULL);
  if (r != 0) {
    uv_fs_req_cleanup(&req);
    return uvwasi__translate_uv_error(r);
  }

  len = strnlen(req.ptr, buf_len);
  if (len >= buf_len) {
    uv_fs_req_cleanup(&req);
    return UVWASI_ENOBUFS;
  }

  memcpy(buf, req.ptr, len);
  buf[len] = '\0';
  *bufused = len + 1;
  uv_fs_req_cleanup(&req);
  return UVWASI_ESUCCESS;
}


uvwasi_errno_t uvwasi_path_remove_directory(uvwasi_t* uvwasi,
                                            uvwasi_fd_t fd,
                                            const char* path,
                                            size_t path_len) {
  char resolved_path[PATH_MAX_BYTES];
  struct uvwasi_fd_wrap_t* wrap;
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

  err = uvwasi__resolve_path(wrap, path, path_len, resolved_path);
  if (err != UVWASI_ESUCCESS)
    return err;

  r = uv_fs_rmdir(NULL, &req, resolved_path, NULL);
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
  char resolved_path[PATH_MAX_BYTES];
  struct uvwasi_fd_wrap_t* wrap;
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

  err = uvwasi__resolve_path(wrap, path, path_len, resolved_path);
  if (err != UVWASI_ESUCCESS)
    return err;

  r = uv_fs_unlink(NULL, &req, resolved_path, NULL);
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
  int r;

  if (uvwasi == NULL)
    return UVWASI_EINVAL;

  r = uvwasi__translate_to_uv_signal(sig);
  if (r == -1)
    return UVWASI_ENOSYS;

  r = uv_kill(uv_os_getpid(), r);
  if (r != 0)
    return uvwasi__translate_uv_error(r);

  return UVWASI_ESUCCESS;
}


uvwasi_errno_t uvwasi_random_get(uvwasi_t* uvwasi, void* buf, size_t buf_len) {
  /* Pending libuv support: https://github.com/libuv/libuv/pull/2347 */
  return UVWASI_ENOTSUP;
}


uvwasi_errno_t uvwasi_sched_yield(uvwasi_t* uvwasi) {
  if (uvwasi == NULL)
    return UVWASI_EINVAL;

#ifndef _WIN32
  if (0 != sched_yield())
    return uvwasi__translate_uv_error(uv_translate_sys_error(errno));
#else
  SwitchToThread();
#endif /* _WIN32 */

  return UVWASI_ESUCCESS;
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
