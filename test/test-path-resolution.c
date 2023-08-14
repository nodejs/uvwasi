#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "uvwasi.h"
#include "../src/fd_table.h"
#include "../src/path_resolver.h"
#include "../src/wasi_rights.h"
#include "test-common.h"

#define BUFFER_SIZE 1024
#define TEST_TMP_DIR "./out/tmp"

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

static uvwasi_errno_t check(char* fd_mp, char* fd_rp, char* path, char** res, uvwasi_lookupflags_t flags) {
  struct uvwasi_fd_wrap_t fd;
  uvwasi_errno_t err;
  uvwasi_size_t len;

  setup_test_environment();

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
  return uvwasi__resolve_path(&uvwasi, &fd, path, len, res, flags);
}

static void pass(char* mp, char* rp, char* path, char* expected) {
  char* resolved;
  char* resolved_follow;
  size_t res_len;
  size_t i;

  assert(UVWASI_ESUCCESS == check(mp, rp, path, &resolved, 0));
  res_len = strlen(resolved);
  assert(res_len == strlen(expected));

  assert(UVWASI_ESUCCESS == check(mp, rp, path, &resolved_follow, UVWASI_LOOKUP_SYMLINK_FOLLOW));
  assert(strlen(resolved_follow) == res_len);

  for (i = 0; i < res_len + 1; i++) {
#ifdef _WIN32
    if (resolved[i] == '\\') {
      assert(resolved_follow[i] == '\\');
      assert(expected[i] == '/');
      continue;
    }
#endif /* _WIN32 */

    assert(resolved[i] == resolved_follow[i]);
    assert(resolved[i] == expected[i]);
  }

  free(resolved);
}

static void pass_follow(char* mp, char* rp, char* path, char* expected) {
  char *resolved;
  size_t res_len;
  size_t i;

  assert(UVWASI_ESUCCESS == check(mp, rp, path, &resolved, UVWASI_LOOKUP_SYMLINK_FOLLOW));
  res_len = strlen(resolved);
  assert(res_len == strlen(expected));

  for (i = 0; i < res_len + 1; i++) {
#ifdef _WIN32
    if (resolved[i] == '\\') {
      assert(resolved_follow[i] == '\\');
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
  char* resolved_follow;

  assert(expected == check(mp, rp, path, &resolved, 0));
  assert(resolved == NULL);

  assert(expected == check(mp, rp, path, &resolved_follow, UVWASI_LOOKUP_SYMLINK_FOLLOW));
  assert(resolved_follow == NULL);
}

static void fail_follow(char *mp, char *rp, char *path, uvwasi_errno_t expected)
{
  char *resolved;

  assert(expected == check(mp, rp, path, &resolved, UVWASI_LOOKUP_SYMLINK_FOLLOW));
  assert(resolved == NULL);
}

static void create_symlink(char* src, char* real_dst) {
  uv_fs_t req;
  int r;

  r = uv_fs_mkdir(NULL, &req, TEST_TMP_DIR, 0777, NULL);
  uv_fs_req_cleanup(&req);
  assert(r == 0 || r == UV_EEXIST);

  r = uv_fs_mkdir(NULL, &req, TEST_TMP_DIR "/dir", 0777, NULL);
  uv_fs_req_cleanup(&req);
  assert(r == 0 || r == UV_EEXIST);

  r = uv_fs_symlink(NULL, &req, src, real_dst, 0, NULL);
  uv_fs_req_cleanup(&req);
  assert(r == 0 || r == UV_EEXIST);
}

int main(void) {
  uvwasi_options_init(&init_options);
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

  /* Arguments: source path, destination real path */
  create_symlink("foo", TEST_TMP_DIR "/bar");
  create_symlink("./foo", TEST_TMP_DIR "/bar2");
  create_symlink("/foo", TEST_TMP_DIR "/bar3");
  create_symlink("../foo", TEST_TMP_DIR "/bar4");
  create_symlink("/../foo", TEST_TMP_DIR "/bar5");
  create_symlink("bar", TEST_TMP_DIR "/baz");
  create_symlink("./bar", TEST_TMP_DIR "/baz2");
  create_symlink("/bar", TEST_TMP_DIR "/baz3");
  create_symlink("../foo", TEST_TMP_DIR "/dir/qux");
  create_symlink("./qux", TEST_TMP_DIR "/dir/quux");

  /* Arguments: fd mapped path, fd real path, path to resolve, expected path */
  pass_follow("/", TEST_TMP_DIR, "/bar", TEST_TMP_DIR "/foo");
  pass_follow("/", TEST_TMP_DIR, "/bar2", TEST_TMP_DIR "/foo");
  pass_follow("/", TEST_TMP_DIR, "/bar3", TEST_TMP_DIR "/foo");
  pass_follow("/", TEST_TMP_DIR, "/bar4", TEST_TMP_DIR "/foo");
  pass_follow("/", TEST_TMP_DIR, "/bar5", TEST_TMP_DIR "/foo");
  pass_follow("/", TEST_TMP_DIR, "/baz", TEST_TMP_DIR "/foo");
  pass_follow("/", TEST_TMP_DIR, "/baz2", TEST_TMP_DIR "/foo");
  pass_follow("/", TEST_TMP_DIR, "/baz3", TEST_TMP_DIR "/foo");
  pass_follow("/", TEST_TMP_DIR, "/dir/qux", TEST_TMP_DIR "/foo");
  pass_follow("/", TEST_TMP_DIR, "/dir/quux", TEST_TMP_DIR "/foo");

  /* Arguments: fd mapped path, fd real path, path to resolve, expected error */
  fail_follow("/dir", TEST_TMP_DIR "/dir", "/dir/qux", UVWASI_ENOTCAPABLE);
  fail_follow("/dir", TEST_TMP_DIR "/dir", "/dir/quux", UVWASI_ENOTCAPABLE);

  uvwasi_destroy(&uvwasi);
  return 0;
}
