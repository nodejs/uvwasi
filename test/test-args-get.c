#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "uvwasi.h"

int main(void) {
  uvwasi_t uvwasi;
  uvwasi_options_t init_options;
  uvwasi_errno_t err;
  uvwasi_size_t argc;
  uvwasi_size_t argv_buf_size;
  char** args_get_argv;
  char* buf;

  init_options.in = 0;
  init_options.out = 1;
  init_options.err = 2;
  init_options.fd_table_size = 3;
  init_options.argc = 3;
  init_options.argv = calloc(3, sizeof(char*));
  init_options.argv[0] = "--foo=bar";
  init_options.argv[1] = "-baz";
  init_options.argv[2] = "100";
  init_options.envp = NULL;
  init_options.preopenc = 0;
  init_options.preopens = NULL;
  init_options.allocator = NULL;

  err = uvwasi_init(&uvwasi, &init_options);
  assert(err == 0);

  err = uvwasi_args_sizes_get(&uvwasi, &argc, &argv_buf_size);
  assert(err == 0);
  assert(argc == init_options.argc);
  assert(argv_buf_size == 19);

  args_get_argv = calloc(argc, sizeof(char*));
  assert(args_get_argv != NULL);
  buf = malloc(argv_buf_size);
  assert(buf != NULL);

  err = uvwasi_args_get(&uvwasi, args_get_argv, buf);
  assert(err == 0);
  assert(strcmp(args_get_argv[0], init_options.argv[0]) == 0);
  assert(strcmp(args_get_argv[1], init_options.argv[1]) == 0);
  assert(strcmp(args_get_argv[2], init_options.argv[2]) == 0);

  uvwasi_destroy(&uvwasi);
  free(init_options.argv);
  free(args_get_argv);
  free(buf);

  return 0;
}
