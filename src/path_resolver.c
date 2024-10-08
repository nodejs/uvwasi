#include <string.h>

#include "uv.h"
#include "uvwasi.h"
#include "uvwasi_alloc.h"
#include "uv_mapping.h"
#include "path_resolver.h"

#define UVWASI__MAX_SYMLINK_FOLLOWS 32

#ifndef _WIN32
# define IS_SLASH(c) ((c) == '/')
#else
# define IS_SLASH(c) ((c) == '/' || (c) == '\\')
#endif /* _WIN32 */


static int uvwasi__is_absolute_path(const char* path, uvwasi_size_t path_len) {
  /* It's expected that only Unix style paths will be generated by WASI. */
  return path != NULL && path_len > 0 && path[0] == '/';
}


static char* uvwasi__strchr_slash(const char* s) {
  /* strchr() that identifies /, as well as \ on Windows. */
  do {
    if (IS_SLASH(*s))
      return (char*) s;
  } while (*s++);

  return NULL;
}

static uvwasi_errno_t uvwasi__combine_paths(const uvwasi_t* uvwasi,
                                            const char* path1,
                                            uvwasi_size_t path1_len,
                                            const char* path2,
                                            uvwasi_size_t path2_len,
                                            char** combined_path,
                                            uvwasi_size_t* combined_len) {
  /* This function joins two paths with '/'. */
  uvwasi_errno_t err;
  char* combined;
  int combined_size;
  int r;

  *combined_path = NULL;
  *combined_len = 0;

  /* The max combined size is the path1 length + the path2 length
     + 2 for a terminating NULL and a possible path separator. */
  combined_size = path1_len + path2_len + 2;
  combined = uvwasi__malloc(uvwasi, combined_size);
  if (combined == NULL) return UVWASI_ENOMEM;

  r = snprintf(combined, combined_size, "%s/%s", path1, path2);
  if (r <= 0) {
    err = uvwasi__translate_uv_error(uv_translate_sys_error(errno));
    goto exit;
  }

  err = UVWASI_ESUCCESS;
  *combined_path = combined;
  *combined_len = strlen(combined);

exit:
  if (err != UVWASI_ESUCCESS) uvwasi__free(uvwasi, combined);
  return err;
}

uvwasi_errno_t uvwasi__normalize_path(const char* path,
                                      uvwasi_size_t path_len,
                                      char* normalized_path,
                                      uvwasi_size_t normalized_len) {
  /* Normalizes path and stores the resulting buffer in normalized_path.
     the sizes of the buffers must correspond to strlen() of the relevant
     buffers, i.e. there must be room in the relevant buffers for a
     NULL-byte. */
  const char* cur;
  char* ptr;
  char* next;
  char* last;
  size_t cur_len;
  int is_absolute;
  int has_trailing_slash;

  if (path_len > normalized_len)
    return UVWASI_ENOBUFS;

  has_trailing_slash = path_len > 0 && IS_SLASH(path[path_len - 1]);

  is_absolute = uvwasi__is_absolute_path(path, path_len);
  normalized_path[0] = '\0';
  ptr = normalized_path;
  for (cur = path; cur != NULL; cur = next + 1) {
    next = uvwasi__strchr_slash(cur);
    cur_len = (next == NULL) ? strlen(cur) : (size_t) (next - cur);

    if (cur_len == 0) {
      if (ptr == normalized_path && next != NULL && is_absolute) {
        *ptr = '/';
        ptr++;
      }

      *ptr = '\0';
    } else if (cur_len == 1 && cur[0] == '.') {
      /* No-op. Just consume the '.' */
    } else if (cur_len == 2 && cur[0] == '.' && cur[1] == '.') {
      /* Identify the path segment that preceded the current one. */
      last = ptr;
      while (!IS_SLASH(*last) && last != normalized_path) {
        last--;
      }

      /* If the result is currently empty, or the last prior path is also '..'
         then output '..'. Otherwise, remove the last path segment. */
      if (ptr == normalized_path ||
          (last == ptr - 2 && last[0] == '.' && last[1] == '.') ||
          (last == ptr - 3 && last[0] == '/' &&
           last[1] == '.' && last[2] == '.')) {
        if (ptr != normalized_path && *(ptr - 1) != '/') {
          *ptr = '/';
          ptr++;
        }

        *ptr = '.';
        ptr++;
        *ptr = '.';
        ptr++;
      } else {
        /* Strip the last segment, but make sure not to strip the '/' if that
           is the entire path. */
        if (last == normalized_path && *last == '/')
          ptr = last + 1;
        else
          ptr = last;
      }

      *ptr = '\0';
    } else {
      if (ptr != normalized_path && *(ptr - 1) != '/') {
        *ptr = '/';
        ptr++;
      }

      memcpy(ptr, cur, cur_len);
      ptr += cur_len;
      *ptr = '\0';
    }

    if (next == NULL)
      break;
  }

  /* Normalized the path to the empty string. Return either '/' or '.'. */
  if (ptr == normalized_path) {
    if (1 == is_absolute)
      *ptr = '/';
    else
      *ptr = '.';

    ptr++;
    *ptr = '\0';
  }

  if (has_trailing_slash && !IS_SLASH(*(ptr - 1))) {
    *ptr = '/';
    ptr++;
    *ptr = '\0';
  }

  return UVWASI_ESUCCESS;
}


static int uvwasi__is_path_sandboxed(const char* path,
                                     uvwasi_size_t path_len,
                                     const char* fd_path,
                                     uvwasi_size_t fd_path_len) {
  char* ptr;
  int remaining_len;

  if (1 == uvwasi__is_absolute_path(fd_path, fd_path_len))
    return path == strstr(path, fd_path) ? 1 : 0;

  /* Handle relative fds that normalized to '.' */
  if ((fd_path_len == 1 && fd_path[0] == '.')
      || (fd_path_len == 2 && fd_path[0] == '.' && fd_path[1] == '/')
  ) {
    /* If the fd's path is '.', then any path does not begin with '..' is OK. */
    if ((path_len == 2 && path[0] == '.' && path[1] == '.') ||
        (path_len > 2 && path[0] == '.' && path[1] == '.' && path[2] == '/')) {
      return 0;
    }

    return 1;
  }

  if (path != strstr(path, fd_path))
    return 0;

  /* Fail if the remaining path starts with '..', '../', '/..', or '/../'. */
  ptr = (char*) path + fd_path_len;
  remaining_len = path_len - fd_path_len;
  if (remaining_len < 2)
    return 1;

  /* Strip a leading slash so the check is only for '..' and '../'. */
  if (*ptr == '/') {
    ptr++;
    remaining_len--;
  }

  if ((remaining_len == 2 && ptr[0] == '.' && ptr[1] == '.') ||
      (remaining_len > 2 && ptr[0] == '.' && ptr[1] == '.' && ptr[2] == '/')) {
    return 0;
  }

  return 1;
}


static uvwasi_errno_t uvwasi__normalize_absolute_path(
                                              const uvwasi_t* uvwasi,
                                              const struct uvwasi_fd_wrap_t* fd,
                                              const char* path,
                                              uvwasi_size_t path_len,
                                              char** normalized_path,
                                              uvwasi_size_t* normalized_len
                                            ) {
  /* This function resolves an absolute path to the provided file descriptor.
     If the file descriptor's path is relative, then this operation will fail
     with UVWASI_ENOTCAPABLE since it doesn't make sense to resolve an absolute
     path to a relative prefix. If the file desciptor's path is also absolute,
     then we just need to verify that the normalized path still starts with
     the file descriptor's path. */
  uvwasi_errno_t err;
  char* abs_path;
  int abs_size;

  *normalized_path = NULL;
  *normalized_len = 0;
  abs_size = path_len + 1;
  abs_path = uvwasi__malloc(uvwasi, abs_size);
  if (abs_path == NULL) {
    err = UVWASI_ENOMEM;
    goto exit;
  }

  /* Normalize the input path first. */
  err = uvwasi__normalize_path(path, path_len, abs_path, path_len);
  if (err != UVWASI_ESUCCESS)
    goto exit;

  /* Once the input is normalized, ensure that it is still sandboxed. */
  if (0 == uvwasi__is_path_sandboxed(abs_path,
                                     path_len,
                                     fd->normalized_path,
                                     strlen(fd->normalized_path))) {
    err = UVWASI_ENOTCAPABLE;
    goto exit;
  }

  *normalized_path = abs_path;
  *normalized_len = abs_size - 1;
  return UVWASI_ESUCCESS;

exit:
  uvwasi__free(uvwasi, abs_path);
  return err;
}


static uvwasi_errno_t uvwasi__normalize_relative_path(
                                              const uvwasi_t* uvwasi,
                                              const struct uvwasi_fd_wrap_t* fd,
                                              const char* path,
                                              uvwasi_size_t path_len,
                                              char** normalized_path,
                                              uvwasi_size_t* normalized_len
                                            ) {
  /* This function resolves a relative path to the provided file descriptor.
     The relative path is concatenated to the file descriptor's path, and then
     normalized. */
  uvwasi_errno_t err;
  char* combined;
  char* normalized = NULL;
  uvwasi_size_t combined_len;
  uvwasi_size_t fd_path_len;
  uvwasi_size_t norm_len;

  *normalized_path = NULL;
  *normalized_len = 0;

  fd_path_len = strlen(fd->normalized_path);

  err = uvwasi__combine_paths(uvwasi,
                              fd->normalized_path,
                              fd_path_len,
                              path,
                              path_len,
                              &combined,
                              &combined_len);
  if (err != UVWASI_ESUCCESS) goto exit;

  normalized = uvwasi__malloc(uvwasi, combined_len + 1);
  if (normalized == NULL) {
    err = UVWASI_ENOMEM;
    goto exit;
  }

  /* Normalize the input path. */
  err = uvwasi__normalize_path(combined,
                               combined_len,
                               normalized,
                               combined_len);
  if (err != UVWASI_ESUCCESS)
    goto exit;

  norm_len = strlen(normalized);

  /* Once the path is normalized, ensure that it is still sandboxed. */
  if (0 == uvwasi__is_path_sandboxed(normalized,
                                     norm_len,
                                     fd->normalized_path,
                                     fd_path_len)) {
    err = UVWASI_ENOTCAPABLE;
    goto exit;
  }

  err = UVWASI_ESUCCESS;
  *normalized_path = normalized;
  *normalized_len = norm_len;

exit:
  if (err != UVWASI_ESUCCESS)
    uvwasi__free(uvwasi, normalized);

  uvwasi__free(uvwasi, combined);
  return err;
}


static uvwasi_errno_t uvwasi__resolve_path_to_host(
                                              const uvwasi_t* uvwasi,
                                              const struct uvwasi_fd_wrap_t* fd,
                                              const char* path,
                                              uvwasi_size_t path_len,
                                              char** resolved_path,
                                              uvwasi_size_t* resolved_len
                                            ) {
  /* Return the normalized path, but resolved to the host's real path.
     `path` must be a NULL-terminated string. */
  char* res_path;
  char* stripped_path;
  int real_path_len;
  int fake_path_len;
  int stripped_len;
#ifdef _WIN32
  uvwasi_size_t i;
#endif /* _WIN32 */

  real_path_len = strlen(fd->real_path);
  fake_path_len = strlen(fd->normalized_path);

  /* If the fake path is '.' just ignore it. */
  if ((fake_path_len == 1 && fd->normalized_path[0] == '.')
      || (fake_path_len == 2
          && fd->normalized_path[0] == '.'
          && fd->normalized_path[1] == '/')
  ) {
    fake_path_len = 0;
  }

  stripped_len = path_len - fake_path_len;

  /* The resolved path's length is calculated as: the length of the fd's real
     path, + 1 for a path separator, and the length of the input path (with the
     fake path stripped off). */
  *resolved_len = stripped_len + real_path_len + 1;
  *resolved_path = uvwasi__malloc(uvwasi, *resolved_len + 1);

  if (*resolved_path == NULL)
    return UVWASI_ENOMEM;

  res_path = *resolved_path;
  stripped_path = (char*) path + fake_path_len;
  memcpy(res_path, fd->real_path, real_path_len);
  res_path += real_path_len;

  if (stripped_len > 1 ||
      (stripped_len == 1 && stripped_path[0] != '/')) {
    if (stripped_path[0] != '/') {
      *res_path = '/';
      res_path++;
    }

    memcpy(res_path, stripped_path, stripped_len);
    res_path += stripped_len;
  }

  *res_path = '\0';

#ifdef _WIN32
  /* Replace / with \ on Windows. */
  res_path = *resolved_path;
  for (i = real_path_len; i < *resolved_len; i++) {
    if (res_path[i] == '/')
      res_path[i] = '\\';
  }
#endif /* _WIN32 */

  return UVWASI_ESUCCESS;
}


uvwasi_errno_t uvwasi__resolve_path(const uvwasi_t* uvwasi,
                                    const struct uvwasi_fd_wrap_t* fd,
                                    const char* path,
                                    uvwasi_size_t path_len,
                                    char** resolved_path,
                                    uvwasi_lookupflags_t flags) {
  uv_fs_t req;
  uvwasi_errno_t err;
  const char* input;
  char* host_path;
  char* normalized_path;
  char* link_target;
  char* normalized_parent;
  char* resolved_link_target;
  uvwasi_size_t input_len;
  uvwasi_size_t host_path_len;
  uvwasi_size_t normalized_len;
  uvwasi_size_t link_target_len;
  uvwasi_size_t normalized_parent_len;
  uvwasi_size_t resolved_link_target_len;
  int follow_count;
  int r;

  input = path;
  input_len = path_len;
  link_target = NULL;
  follow_count = 0;
  host_path = NULL;
  normalized_parent = NULL;
  resolved_link_target = NULL;

  if (uvwasi__is_absolute_path(input, input_len)) {
    *resolved_path = NULL;
    return UVWASI_ENOTCAPABLE;
  }

start:
  normalized_path = NULL;
  err = UVWASI_ESUCCESS;

  if (input_len != strnlen(input, input_len - 1) + 1) {
    err = UVWASI_EINVAL;
    goto exit;
  }

  if (1 == uvwasi__is_absolute_path(input, input_len)) {
    err = uvwasi__normalize_absolute_path(uvwasi,
                                          fd,
                                          input,
                                          input_len,
                                          &normalized_path,
                                          &normalized_len);
  } else {
    err = uvwasi__normalize_relative_path(uvwasi,
                                          fd,
                                          input,
                                          input_len,
                                          &normalized_path,
                                          &normalized_len);
  }

  if (err != UVWASI_ESUCCESS)
    goto exit;

  uvwasi__free(uvwasi, host_path);
  err = uvwasi__resolve_path_to_host(uvwasi,
                                     fd,
                                     normalized_path,
                                     normalized_len,
                                     &host_path,
                                     &host_path_len);
  if (err != UVWASI_ESUCCESS)
    goto exit;

  if ((flags & UVWASI_LOOKUP_SYMLINK_FOLLOW) == UVWASI_LOOKUP_SYMLINK_FOLLOW) {
    r = uv_fs_readlink(NULL, &req, host_path, NULL);

    if (r != 0) {
#ifdef _WIN32
      /* uv_fs_readlink() returns UV__UNKNOWN on Windows. Try to get a better
         error using uv_fs_stat(). */
      if (r == UV__UNKNOWN) {
        uv_fs_req_cleanup(&req);
        r = uv_fs_stat(NULL, &req, host_path, NULL);

        if (r == 0) {
          if (uvwasi__stat_to_filetype(&req.statbuf) !=
              UVWASI_FILETYPE_SYMBOLIC_LINK) {
            r = UV_EINVAL;
          }
        }

        /* Fall through. */
      }
#endif /* _WIN32 */

      /* Don't report UV_EINVAL or UV_ENOENT. They mean that either the file
          does not exist, or it is not a symlink. Both are OK. */
      if (r != UV_EINVAL && r != UV_ENOENT)
        err = uvwasi__translate_uv_error(r);

      uv_fs_req_cleanup(&req);
      goto exit;
    }

    /* Clean up memory and follow the link, unless it's time to return ELOOP. */
    follow_count++;
    if (follow_count >= UVWASI__MAX_SYMLINK_FOLLOWS) {
      uv_fs_req_cleanup(&req);
      err = UVWASI_ELOOP;
      goto exit;
    }

    link_target_len = strlen(req.ptr);
    uvwasi__free(uvwasi, link_target);
    link_target = uvwasi__malloc(uvwasi, link_target_len + 1);
    if (link_target == NULL) {
      uv_fs_req_cleanup(&req);
      err = UVWASI_ENOMEM;
      goto exit;
    }

    memcpy(link_target, req.ptr, link_target_len + 1);
    uv_fs_req_cleanup(&req);

    if (1 == uvwasi__is_absolute_path(link_target, link_target_len)) {
      input = link_target;
      input_len = link_target_len;
    } else {
      uvwasi__free(uvwasi, normalized_parent);
      uvwasi__free(uvwasi, resolved_link_target);

      err = uvwasi__combine_paths(uvwasi,
                                  normalized_path,
                                  normalized_len,
                                  "..",
                                  2,
                                  &normalized_parent,
                                  &normalized_parent_len);
      if (err != UVWASI_ESUCCESS) goto exit;
      err = uvwasi__combine_paths(uvwasi,
                                  normalized_parent,
                                  normalized_parent_len,
                                  link_target,
                                  link_target_len,
                                  &resolved_link_target,
                                  &resolved_link_target_len);
      if (err != UVWASI_ESUCCESS) goto exit;

      input = resolved_link_target;
      input_len = resolved_link_target_len;
    }

    uvwasi__free(uvwasi, normalized_path);
    goto start;
  }

exit:
  if (err == UVWASI_ESUCCESS) {
    *resolved_path = host_path;
  } else {
    *resolved_path = NULL;
    uvwasi__free(uvwasi, host_path);
  }

  uvwasi__free(uvwasi, link_target);
  uvwasi__free(uvwasi, normalized_path);
  uvwasi__free(uvwasi, normalized_parent);
  uvwasi__free(uvwasi, resolved_link_target);

  return err;
}
