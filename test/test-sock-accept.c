#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "uv.h"
#include "uvwasi.h"
#include "wasi_serdes.h"

#define PREOPEN_SOCK 3
#define INVALID_SOCK 42
#define DEFAULT_BACKLOG 5

#define TEST_PORT_1 10500

void on_client_connect(uv_connect_t * req, int status) {
  if (status < 0) {
      fprintf(stderr, "New connection error %s\n", uv_strerror(status));
      // error!
      return;
  }
  uv_close((uv_handle_t *)req->handle, NULL);
  free(req);
}

void makeClientConnection(uv_loop_t* loop) {
  int r = 0;
  uv_tcp_t* socket = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(loop, socket);
  uv_connect_t* connect = (uv_connect_t*)malloc(sizeof(uv_connect_t));
  struct sockaddr_in dest;
  r = uv_ip4_addr("127.0.0.1", TEST_PORT_1, &dest);
  assert(r == 0);

  r = uv_tcp_connect(connect, socket, (const struct sockaddr*)&dest, on_client_connect);
  assert(r == 0);
  uv_run(loop, UV_RUN_NOWAIT);
  free(socket);
}

int main(void) {
#if !defined(_WIN32) && !defined(__ANDROID__)
  uv_loop_t *loop = uv_default_loop();
  uvwasi_t uvwasi;
  uvwasi_options_t init_options;
  uvwasi_errno_t err = 0;

  uvwasi_options_init(&init_options);
  init_options.preopen_socketc = 1;
  init_options.preopen_sockets = calloc(1, sizeof(uvwasi_preopen_socket_t));
  init_options.preopen_sockets->address = "0.0.0.0";
  init_options.preopen_sockets->port = 10500;

  err = uvwasi_init(&uvwasi, &init_options);
  assert(err == 0);

  // validate when we ask for an invalid socket
  uvwasi_fd_t fd;
  err = uvwasi_sock_accept(&uvwasi, INVALID_SOCK, 0, &fd);
  assert(err == UVWASI_EBADF);

  // validate the case where there is no connection and we are not
  // blocking
  err = uvwasi_sock_accept(&uvwasi, PREOPEN_SOCK, UVWASI_FDFLAG_NONBLOCK, &fd);
  assert(err == UVWASI_EAGAIN);

  // make a connection to the server
  makeClientConnection(loop);

  // validate case where there is a pending connection when we do a sock
  // accept
  err = uvwasi_sock_accept(&uvwasi, PREOPEN_SOCK, 0, &fd);
  assert(err == 0);
  assert(fd != 0);

  uvwasi_destroy(&uvwasi);
  free(init_options.preopen_sockets);
#endif /* !defined(_WIN32) && !defined(__ANDROID__) */
  return 0;
}
