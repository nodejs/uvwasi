#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "uvwasi.h"
#include "../src/fd_table.h"
#include "../src/path_resolver.h"
#include "../src/wasi_rights.h"

#define BUFFER_SIZE 1024

static uvwasi_t uvwasi;
static uvwasi_options_t init_options;
static char buffer[BUFFER_SIZE];
static char normalized_path_buffer[BUFFER_SIZE];

static void check_normalize(char* path, char* expected) {
  uvwasi_errno_t err;
  char* normalized;

  buffer[0] = '\0';
  normalized = buffer;
  err = uvwasi__normalize_path(path, strlen(path), normalized, BUFFER_SIZE);
  assert(UVWASI_ESUCCESS == err);
  assert(0 == strcmp(buffer, expected));
}

static uvwasi_errno_t check(char* fd_mp, char* fd_rp, char* path, char** res) {
  struct uvwasi_fd_wrap_t fd;
  uvwasi_errno_t err;
  uvwasi_size_t len;

  len = strlen(path);
  fd.id = 3;
  fd.fd = 3;
  fd.path = fd_mp;
  fd.real_path = fd_rp;
  fd.normalized_path = normalized_path_buffer;
  fd.type = UVWASI_FILETYPE_DIRECTORY;
  fd.rights_base = UVWASI__RIGHTS_ALL;
  fd.rights_inheriting = UVWASI__RIGHTS_ALL;
  fd.preopen = 0;
  err = uvwasi__normalize_path(fd_mp,
                               strlen(fd_mp),
                               fd.normalized_path,
                               strlen(fd_mp));
  if (err != UVWASI_ESUCCESS)
    return err;
  return uvwasi__resolve_path(&uvwasi, &fd, path, len, res, 0);
}

static void pass(char* mp, char* rp, char* path, char* expected) {
  char* resolved;
  size_t res_len;
  size_t i;

  assert(UVWASI_ESUCCESS == check(mp, rp, path, &resolved));
  res_len = strlen(resolved);
  assert(res_len == strlen(expected));

  for (i = 0; i < res_len + 1; i++) {
#ifdef _WIN32
    if (resolved[i] == '\\') {
      assert(expected[i] == '/');
      continue;
    }
#endif /* _WIN32 */

    assert(resolved[i] == expected[i]);
  }

  free(resolved);
}

static void fail(char* mp, char* rp, char* path, uvwasi_errno_t expected) {
  char* resolved;

  assert(expected == check(mp, rp, path, &resolved));
  assert(resolved == NULL);
}

int main(void) {
  uvwasi_errno_t err;

  init_options.in = 0;
  init_options.out = 1;
  init_options.err = 2;
  init_options.fd_table_size = 3;
  init_options.argc = 0;
  init_options.argv = NULL;
  init_options.envp = NULL;
  init_options.preopenc = 0;
  init_options.preopens = NULL;
  init_options.allocator = NULL;
  assert(0 == uvwasi_init(&uvwasi, &init_options));

  /* Arguments: input path, expected normalized path */
  check_normalize("", ".");
  check_normalize(".", ".");
  check_normalize("./", ".");
  check_normalize("./.", ".");
  check_normalize("./..", "..");
  check_normalize("./../", "..");
  check_normalize("..", "..");
  check_normalize("../", "..");
  check_normalize("../.", "..");
  check_normalize("../..", "../..");
  check_normalize("/", "/");
  check_normalize("./foo", "foo");
  check_normalize("./foo/../bar", "bar");
  check_normalize("./foo/..bar", "foo/..bar");
  check_normalize("/foo/./bar", "/foo/bar");
  check_normalize("/foo/baz/../bar", "/foo/bar");
  check_normalize("/foo/../bar", "/bar");
  check_normalize("/../bar", "/bar");
  check_normalize("/../../../bar", "/bar");
  check_normalize("/../../../bar/", "/bar");
  check_normalize("/../../../", "/");
  check_normalize("////..//../..///", "/");
  check_normalize("./foo//", "foo");
  check_normalize("./foo/////bar", "foo/bar");
  check_normalize("//", "/");
  check_normalize("..//", "..");
  check_normalize(".//", ".");
  check_normalize("./foo/bar/baz/../../../..", "..");
  check_normalize("./foo/bar/baz/../../../../", "..");
  check_normalize("./foo/bar/baz/../../../../..", "../..");
  check_normalize("./foo/bar/baz/../../../../../", "../..");
  check_normalize("../../../test_path", "../../../test_path");
  check_normalize("./././test_path", "test_path");

  /* Arguments: fd mapped path, fd real path, path to resolve, expected path */
  pass("/", "/foo", "test_path", "/foo/test_path");
  pass("/", "/foo", "/test_path", "/foo/test_path");
  pass("/bar", "/baz", "test_path", "/baz/test_path");
  pass("/bar", "/baz", "./test_path", "/baz/test_path");
  pass("/bar", "/baz", "../bar/test_path", "/baz/test_path");
  pass("/bar", "/baz", "../bar/./test_path/../test_path", "/baz/test_path");
  pass("/bar", "/baz", "/bar/test_path", "/baz/test_path");
  pass("/bar", "/baz", "/bar/../bar/test_path", "/baz/test_path");
  pass(".", "/foo", "test_path", "/foo/test_path");
  pass("./", "/foo", "test_path", "/foo/test_path");
  pass(".", "/foo", "./test_path", "/foo/test_path");
  pass("./", "/foo", "./test_path", "/foo/test_path");
  pass(".", "/foo", "bar/../test_path", "/foo/test_path");
  pass("foo", "/baz", "../foo/test_path", "/baz/test_path");
  pass("..", "/foo", "test_path", "/foo/test_path");
  pass("../baz", "/foo", "test_path", "/foo/test_path");
  pass("../baz", "/foo", "./test_path", "/foo/test_path");
  pass("../baz", "/foo", "../baz/test_path", "/foo/test_path");

  /* Arguments: fd mapped path, fd real path, path to resolve, expected error */
  fail("/bar", "/baz", "/test_path", UVWASI_ENOTCAPABLE);
  fail("/bar", "/baz", "/bar/../test_path", UVWASI_ENOTCAPABLE);
  fail(".", "/baz", "bar/../../test_path", UVWASI_ENOTCAPABLE);
  fail("..", "/baz", "../../test_path", UVWASI_ENOTCAPABLE);
  fail("foo", "/baz", "../abc/test_path", UVWASI_ENOTCAPABLE);
  fail("foo", "/baz", "../../foo/test_path", UVWASI_ENOTCAPABLE);
  fail("../baz", "/foo", "../bak/test_path", UVWASI_ENOTCAPABLE);

  uvwasi_destroy(&uvwasi);
  return 0;
}
