#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "uvwasi.h"
#include "uv.h"
#include "test-common.h"

#define TEST_TMP_DIR "./out/tmp"
#define SECRET_FILE "secret"
#define SECRET_CONTENT "I am a password"
#define TEST_DIR "tdir"
#define BUFFER_SIZE 1024

void setup() {
    uv_fs_t req;
    int r;

    // write SECRET_CONTENT to SECRET_FILE
    char path[128];
    snprintf(path, sizeof(path), "%s/%s", TEST_TMP_DIR, SECRET_FILE);
    r = uv_fs_open(NULL, &req, path, O_WRONLY | O_CREAT, 0644, NULL);
    uv_fs_req_cleanup(&req);
    assert(r >= 0 || r == UV_EEXIST);

    int secret_fd = r;
    uv_buf_t buf = uv_buf_init((char*)SECRET_CONTENT, strlen(SECRET_CONTENT));
    r = uv_fs_write(NULL, &req, secret_fd, &buf, 1, -1, NULL);
    uv_fs_req_cleanup(&req);
    assert(r == (int)strlen(SECRET_CONTENT));

    r = uv_fs_close(NULL, &req, secret_fd, NULL);
    uv_fs_req_cleanup(&req);
    assert(r == 0);

    snprintf(path, sizeof(path), "%s/%s", TEST_TMP_DIR, TEST_DIR);
    r = uv_fs_mkdir(NULL, &req, path, 0777, NULL);
    uv_fs_req_cleanup(&req);
    assert(r == 0 || r == UV_EEXIST);
}

void teardown() {
    uv_fs_t req;
    int r;

    char path[128];
    snprintf(path, sizeof(path), "%s/%s", TEST_TMP_DIR, SECRET_FILE);
    r = uv_fs_unlink(NULL, &req, path, NULL);
    uv_fs_req_cleanup(&req);
    assert(r == 0 || r == UV_ENOENT);

    snprintf(path, sizeof(path), "%s/%s", TEST_TMP_DIR, TEST_DIR);
    r = uv_fs_rmdir(NULL, &req, path, NULL);
    uv_fs_req_cleanup(&req);
    assert(r == 0 || r == UV_ENOENT);
}

int main(void) {
    const char* target_path = "../secret";
    const char* linkname = "./invalid_symlink.txt";
    char preopen_path[128];
    uvwasi_t uvwasi;
    uvwasi_options_t init_options;
    uvwasi_errno_t err;
    uv_fs_t req;
    int r;

    setup_test_environment();

    r = uv_fs_mkdir(NULL, &req, TEST_TMP_DIR, 0777, NULL);
    uv_fs_req_cleanup(&req);
    assert(r == 0 || r == UV_EEXIST);

    setup();

    snprintf(preopen_path, sizeof(preopen_path), "%s/%s", TEST_TMP_DIR, TEST_DIR);

    uvwasi_options_init(&init_options);
    init_options.preopenc = 1;
    init_options.preopens = calloc(1, sizeof(uvwasi_preopen_t));
    init_options.preopens[0].mapped_path = preopen_path;
    init_options.preopens[0].real_path = preopen_path;

    err = uvwasi_init(&uvwasi, &init_options);
    assert(err == 0);

    // Attempt to create a symlink with a target path that resolves to the parent directory
    err = uvwasi_path_symlink(&uvwasi,
                              target_path,
                              strlen(target_path),
                              3,
                              linkname,
                              strlen(linkname));

    // Assert that the operation fails
    // Expecting an error due to invalid target path
    assert(err == UVWASI_ENOTCAPABLE);

    free(init_options.preopens);
    uvwasi_destroy(&uvwasi);

    teardown();
    return 0;
}