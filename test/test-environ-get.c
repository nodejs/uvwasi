#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "uvwasi.h"

#ifdef __APPLE__
# include <crt_externs.h>
# define environ (*_NSGetEnviron())
#elif !defined(_MSC_VER)
extern char** environ;
#endif

int main(void) {
  uvwasi_t uvwasi;
  uvwasi_options_t init_options;
  uvwasi_errno_t err;
  uvwasi_size_t envc;
  uvwasi_size_t env_buf_size;
  char** env_get_env;
  char* buf;
  uvwasi_size_t i;

  init_options.in = 0;
  init_options.out = 1;
  init_options.err = 2;
  init_options.fd_table_size = 3;
  init_options.argc = 0;
  init_options.argv = NULL;
  init_options.envp = (const char**) environ;
  init_options.preopenc = 0;
  init_options.preopens = NULL;
  init_options.allocator = NULL;

  err = uvwasi_init(&uvwasi, &init_options);
  assert(err == 0);

  err = uvwasi_environ_sizes_get(&uvwasi, &envc, &env_buf_size);
  assert(err == 0);
  assert(envc > 0);
  assert(env_buf_size > 0);
  buf = malloc(env_buf_size);
  assert(buf != NULL);

  env_get_env = calloc(envc, sizeof(char*));
  assert(env_get_env != NULL);
  err = uvwasi_environ_get(&uvwasi, env_get_env, buf);
  assert(err == 0);

  uvwasi_destroy(&uvwasi);
  free(buf);
  free(env_get_env);

  return 0;
}
