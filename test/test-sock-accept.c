#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "uv.h"
#include "uvwasi.h"
#include "wasi_serdes.h"

#define PREOPEN_SOCK 3
#define INVALID_SOCK 42

#define CONNECT_ADDRESS "127.0.0.1"
#define TEST_PORT_1 10500
#define CONNECTION_WAIT_TIME 2000
int delayedThreadTime = 4000;
int immediateThreadTime = 0;

void on_uv_close(uv_handle_t* handle) {
  free(handle);
}

void on_client_connect(uv_connect_t * req, int status) {
  if (status < 0) {
      fprintf(stderr, "New connection error %s\n", uv_strerror(status));
      // error!
      return;
  }
  uv_close((uv_handle_t *)req->handle, on_uv_close);
  free(req);
}

void client_connection_thread(void* time) {
  int r = 0;
  uv_loop_t loop;
  uv_loop_init(&loop);

  uv_sleep(*((int*) time));

  uv_tcp_t* socket = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(&loop, socket);
  uv_connect_t* connect = (uv_connect_t*)malloc(sizeof(uv_connect_t));
  struct sockaddr_in dest;
  r = uv_ip4_addr(CONNECT_ADDRESS, TEST_PORT_1, &dest);
  assert(r == 0);
  r = uv_tcp_connect(connect, socket, (const struct sockaddr*)&dest, on_client_connect);
  assert(r == 0);

  uv_run(&loop, UV_RUN_NOWAIT);
  uv_loop_close(&loop);
}

void start_client_connection_thread(int* time) {
  uv_thread_t delayed_thread;
  uv_thread_create(&delayed_thread, client_connection_thread, time);
}

int main(void) {
  uvwasi_t uvwasi;
  uvwasi_options_t init_options;
  uvwasi_errno_t err = 0;

  uvwasi_options_init(&init_options);
  init_options.preopen_socketc = 1;
  init_options.preopen_sockets = calloc(1, sizeof(uvwasi_preopen_socket_t));
  init_options.preopen_sockets->address = CONNECT_ADDRESS;
  init_options.preopen_sockets->port = TEST_PORT_1;

  err = uvwasi_init(&uvwasi, &init_options);
  assert(err == 0);
  // validate when we ask for an invalid socket
  uvwasi_fd_t fd;
  err = uvwasi_sock_accept(&uvwasi, INVALID_SOCK, 0, &fd);
  assert(err == UVWASI_EBADF);

  // validate when we ask for an invalid option
  err = uvwasi_sock_accept(&uvwasi, TEST_PORT_1, UVWASI_FDFLAG_APPEND, &fd);
  assert(err == UVWASI_ENOTSUP);
  err = uvwasi_sock_accept(&uvwasi, TEST_PORT_1, UVWASI_FDFLAG_DSYNC, &fd);
  assert(err == UVWASI_ENOTSUP);
  err = uvwasi_sock_accept(&uvwasi, TEST_PORT_1, UVWASI_FDFLAG_RSYNC, &fd);
  assert(err == UVWASI_ENOTSUP);
  err = uvwasi_sock_accept(&uvwasi, TEST_PORT_1, UVWASI_FDFLAG_SYNC, &fd);
  assert(err == UVWASI_ENOTSUP);

  // validate the case where there is no connection and we are not
  // blocking
  err = uvwasi_sock_accept(&uvwasi, PREOPEN_SOCK, UVWASI_FDFLAG_NONBLOCK, &fd);
  assert(err == UVWASI_EAGAIN);

  // validate case where there is a pending connection when we do a sock
  // accept
  start_client_connection_thread(&immediateThreadTime);
  uv_sleep(CONNECTION_WAIT_TIME);
  err = uvwasi_sock_accept(&uvwasi, PREOPEN_SOCK, 0, &fd);
  assert(err == 0);
  assert(fd != 0);
  err = uvwasi_fd_close(&uvwasi, fd);
  assert(err == 0);

  // validate case where there is no connection when we do a sock accept
  // but one comes in afterwards
  start_client_connection_thread(&delayedThreadTime);
  err = uvwasi_sock_accept(&uvwasi, PREOPEN_SOCK, UVWASI_FDFLAG_NONBLOCK, &fd);
  assert(err == UVWASI_EAGAIN);
  err = uvwasi_sock_accept(&uvwasi, PREOPEN_SOCK, 0, &fd);
  assert(err == 0);
  assert(fd != 0);
  err = uvwasi_fd_close(&uvwasi, fd);
  assert(err == 0);

  // validate case where one accept may queue while one is being handled
  start_client_connection_thread(&delayedThreadTime);
  start_client_connection_thread(&delayedThreadTime);
  err = uvwasi_sock_accept(&uvwasi, PREOPEN_SOCK, UVWASI_FDFLAG_NONBLOCK, &fd);
  assert(err == UVWASI_EAGAIN);
  err = uvwasi_sock_accept(&uvwasi, PREOPEN_SOCK, 0, &fd);
  assert(err == 0);
  assert(fd != 0);
  err = uvwasi_fd_close(&uvwasi, fd);
  assert(err == 0);
  err = uvwasi_sock_accept(&uvwasi, PREOPEN_SOCK, 0, &fd);
  assert(err == 0);
  assert(fd != 0);
  err = uvwasi_fd_close(&uvwasi, fd);
  assert(err == 0);

  // validate two accepts queue up properly
  start_client_connection_thread(&immediateThreadTime);
  start_client_connection_thread(&immediateThreadTime);
  start_client_connection_thread(&immediateThreadTime);
  uv_sleep(CONNECTION_WAIT_TIME);
  // wait for a connection and then close the fd
  // closing the fd runs the event loop which triggers
  // the connect callback such that we not have
  // to wait for the next accept
  err = uvwasi_sock_accept(&uvwasi, PREOPEN_SOCK, 0, &fd);
  assert(err == 0);
  err = uvwasi_fd_close(&uvwasi, fd);
  assert(err == 0);
  err = uvwasi_sock_accept(&uvwasi, PREOPEN_SOCK, UVWASI_FDFLAG_NONBLOCK, &fd);
  assert(err == 0);
  assert(fd != 0);
  err = uvwasi_fd_close(&uvwasi, fd);
  assert(err == 0);
  err = uvwasi_sock_accept(&uvwasi, PREOPEN_SOCK, UVWASI_FDFLAG_NONBLOCK, &fd);
  assert(err == 0);
  assert(fd != 0);
  err = uvwasi_fd_close(&uvwasi, fd);
  assert(err == 0);

  err = uvwasi_fd_close(&uvwasi, PREOPEN_SOCK);
  assert(err == 0);

  uvwasi_destroy(&uvwasi);
  free(init_options.preopen_sockets);
  return 0;
}
