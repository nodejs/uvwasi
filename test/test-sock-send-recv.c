#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "uv.h"
#include "uvwasi.h"
#include "wasi_serdes.h"
#include <stdio.h>

#define PREOPEN_SOCK 3
#define INVALID_SOCK 42
#define DEFAULT_BACKLOG 5

static int immediate_thread_time = 0;

#define CONNECT_ADDRESS "127.0.0.1"
#define TEST_PORT_1 10500

void on_uv_close(uv_handle_t* handle) {
  free(handle);
}

static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  buf->base = malloc(suggested_size);
  buf->len = suggested_size;
}

void echo_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
  if (nread > 0) {
    uv_buf_t send_buf;
    uv_read_stop(stream);
    send_buf.base = buf->base;
    send_buf.len = nread;
    uv_try_write(stream, &send_buf, 1);
  }
  uv_close((uv_handle_t *) stream, on_uv_close);
  free(buf->base);
}

void on_client_connect(uv_connect_t * req, int status) {
  if (status < 0) {
    fprintf(stderr, "New connection error %s\n", uv_strerror(status));
    // error!
    return;
  }
  uv_read_start((uv_stream_t*) req->handle, alloc_cb, echo_read);
  free(req);
}

void client_connection_echo_thread(void* time) {
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

  uv_run(&loop, UV_RUN_DEFAULT);
  uv_loop_close(&loop);
}

void start_client_connection_echo_thread(int* time) {
  uv_thread_t delayed_thread;
  uv_thread_create(&delayed_thread, client_connection_echo_thread, time);
}

int main(void) {
  uvwasi_t uvwasi;
  uvwasi_options_t init_options;
  uvwasi_errno_t err = 0;
  uvwasi_ciovec_t send_ciovecs[2];
  char send_ciovecs_buffers[2][4];
  uvwasi_iovec_t recv_iovecs[1];
  char recv_iovecs_buffers[1][1000];
  void* send_buf;
  uvwasi_size_t nio;
  uvwasi_preopen_socket_t preopen_sock;

  uvwasi_options_init(&init_options);
  init_options.preopen_socketc = 1;
  init_options.preopen_sockets = &preopen_sock;
  init_options.preopen_sockets->address = CONNECT_ADDRESS;
  init_options.preopen_sockets->port = TEST_PORT_1;

  err = uvwasi_init(&uvwasi, &init_options);
  assert(err == 0);

  // validate we can send data
  uvwasi_fd_t fd;
  start_client_connection_echo_thread(&immediate_thread_time);
  err = uvwasi_sock_accept(&uvwasi, PREOPEN_SOCK, 0, &fd);
  assert(err == 0);
  assert(fd != 0);
  nio = 0;
  for (uvwasi_size_t i = 0; i < 2; ++i) {
    send_ciovecs[i].buf_len = 4;
    send_buf = send_ciovecs_buffers[i];
    assert(send_buf != NULL);
    memset(send_buf, 0, send_ciovecs[i].buf_len);
    if (i == 0) {
      memcpy(send_buf,"hihi",4);
    } else { 
      memcpy(send_buf,"hih",3);
    }
    send_ciovecs[i].buf = send_buf;
  }
  err = uvwasi_sock_send(&uvwasi, fd, send_ciovecs, 2, 0, &nio);
  assert(err == 0);
  assert(nio == 8);

  uvwasi_size_t received_len = 0;
  uvwasi_roflags_t out_flags;

  recv_iovecs[0].buf_len = 1000; 
  recv_iovecs[0].buf = recv_iovecs_buffers[0];
  err = uvwasi_sock_recv(&uvwasi, fd, recv_iovecs, 1, 0, &received_len, &out_flags);
  assert(err == 0);
  assert(received_len == 8);
  assert(strcmp(recv_iovecs[0].buf, "hihihih") == 0);
  err = uvwasi_fd_close(&uvwasi, fd);
  assert(err == 0);

  // validate we get expected error trying to send after socket shutdown
  start_client_connection_echo_thread(&immediate_thread_time);
  err = uvwasi_sock_accept(&uvwasi, PREOPEN_SOCK, 0, &fd);
  assert(err == 0);
  err = uvwasi_sock_shutdown(&uvwasi, fd, UVWASI_SHUT_WR);
  assert(err == 0);
  err = uvwasi_sock_send(&uvwasi, fd, send_ciovecs, 2, 0, &nio);
  assert(err != 0);
  assert(err == UVWASI_EPIPE);

  // clean up
  err = uvwasi_fd_close(&uvwasi, fd);
  assert(err == 0);
  err = uvwasi_fd_close(&uvwasi, PREOPEN_SOCK);
  assert(err == 0);
  uvwasi_destroy(&uvwasi);
  return 0;
}
