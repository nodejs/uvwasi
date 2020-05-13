#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "wasi_serdes.h"

void test_bound_checks(void);
void test_basic_types(void);
void test_fdstat_t(void);
void test_filestat_t(void);
void test_prestat_t(void);
void test_event_t(void);
void test_subscription_t(void);

int main(void) {
  test_bound_checks();
  test_basic_types();
  test_fdstat_t();
  test_filestat_t();
  test_prestat_t();
  test_event_t();
  test_subscription_t();
  return 0;
}

/* First 16 digits of PI, used to make sure that the algorithm doesn't overwrite
   anything it shouldn't. */
const char canary[16] = { 3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 9, 7, 9, 3 };

#define ADD_CANARIES(type) ((UVWASI_SERDES_SIZE_##type) + 2 * sizeof(canary))

/* Writes the canary to the beginning and end of the buffer. */
void use_canaries(char* ptr, size_t size) {
  assert(size >= 2 * sizeof(canary));
  memcpy(ptr, canary, sizeof(canary));
  memcpy(ptr + size - sizeof(canary), canary, sizeof(canary));
}

/* Checks that the canaries at the beginning and end of the buffer
   are intact. */
void check_canaries(const char* ptr, size_t size) {
  assert(size >= 2 * sizeof(canary));
  assert(memcmp(ptr, canary, sizeof(canary)) == 0);
  assert(memcmp(ptr + size - sizeof(canary), canary, sizeof(canary)) == 0);
}

void test_bound_checks(void) {
  /* Regardless of the type, the macro should catch negative offsets
     and sizes. */
  assert(!UVWASI_SERDES_CHECK_BOUNDS(-500, 1000, uint8_t));
  assert(!UVWASI_SERDES_CHECK_BOUNDS(-500, -100, uint16_t));
  assert(!UVWASI_SERDES_CHECK_BOUNDS(5000, 1000, uint32_t));
  assert(!UVWASI_SERDES_CHECK_BOUNDS(-500, 1000, uint64_t));
  assert(!UVWASI_SERDES_CHECK_BOUNDS(-500, -100, event_t));
  assert(!UVWASI_SERDES_CHECK_BOUNDS(5000, 1000, fdstat_t));
  assert(!UVWASI_SERDES_CHECK_ARRAY_BOUNDS(0, 1000, filestat_t, -1));
  /* This causes an integer overflow, which should be detected correctly. */
  assert(!UVWASI_SERDES_CHECK_ARRAY_BOUNDS(0, 0xffffffffffffffffllu,
                                           subscription_t,
                                           0xffffffffffffffffllu));

  assert(UVWASI_SERDES_CHECK_BOUNDS(19, 20, uint8_t));
  assert(!UVWASI_SERDES_CHECK_BOUNDS(20, 20, uint8_t));
  assert(UVWASI_SERDES_CHECK_BOUNDS(18, 20, uint16_t));
  assert(!UVWASI_SERDES_CHECK_BOUNDS(19, 20, uint16_t));
  assert(UVWASI_SERDES_CHECK_BOUNDS(16, 20, uint32_t));
  assert(!UVWASI_SERDES_CHECK_BOUNDS(17, 20, uint32_t));
  assert(UVWASI_SERDES_CHECK_BOUNDS(12, 20, uint64_t));
  assert(!UVWASI_SERDES_CHECK_BOUNDS(13, 20, uint64_t));
  assert(UVWASI_SERDES_CHECK_BOUNDS(0, 24, fdstat_t));
  assert(!UVWASI_SERDES_CHECK_BOUNDS(1, 24, fdstat_t));
  assert(UVWASI_SERDES_CHECK_BOUNDS(0, 64, filestat_t));
  assert(!UVWASI_SERDES_CHECK_BOUNDS(1, 64, filestat_t));
  assert(UVWASI_SERDES_CHECK_BOUNDS(0, 8, prestat_t));
  assert(!UVWASI_SERDES_CHECK_BOUNDS(1, 8, prestat_t));
  assert(UVWASI_SERDES_CHECK_BOUNDS(0, 32, event_t));
  assert(!UVWASI_SERDES_CHECK_BOUNDS(1, 32, event_t));
  assert(UVWASI_SERDES_CHECK_BOUNDS(0, 48, subscription_t));
  assert(!UVWASI_SERDES_CHECK_BOUNDS(1, 48, subscription_t));

  assert(UVWASI_SERDES_CHECK_ARRAY_BOUNDS(0, 480, subscription_t, 10));
  assert(!UVWASI_SERDES_CHECK_ARRAY_BOUNDS(1, 480, subscription_t, 10));
  assert(UVWASI_SERDES_CHECK_ARRAY_BOUNDS(0, 8000, inode_t, 1000));
  assert(!UVWASI_SERDES_CHECK_ARRAY_BOUNDS(1, 8000, inode_t, 1000));
}

void test_basic_types(void) {
  {
    char buf[ADD_CANARIES(uint8_t)];
    use_canaries(buf, sizeof(buf));
    uvwasi_serdes_write_uint8_t(buf, sizeof(canary), 0xabu);
    check_canaries(buf, sizeof(buf));
    assert(uvwasi_serdes_read_uint8_t(buf, sizeof(canary)) == 0xabu);
  }
  {
    char buf[ADD_CANARIES(uint16_t)];
    use_canaries(buf, sizeof(buf));
    uvwasi_serdes_write_uint16_t(buf, sizeof(canary), 0xabcdu);
    check_canaries(buf, sizeof(buf));
    assert(uvwasi_serdes_read_uint16_t(buf, sizeof(canary)) == 0xabcdu);
  }
  {
    char buf[ADD_CANARIES(uint32_t)];
    use_canaries(buf, sizeof(buf));
    uvwasi_serdes_write_uint32_t(buf, sizeof(canary), 0xabcdef01u);
    check_canaries(buf, sizeof(buf));
    assert(uvwasi_serdes_read_uint32_t(buf, sizeof(canary)) == 0xabcdef01u);
  }
  {
    char buf[ADD_CANARIES(uint64_t)];
    use_canaries(buf, sizeof(buf));
    uvwasi_serdes_write_uint64_t(buf, sizeof(canary), 0xabcdef0123llu);
    check_canaries(buf, sizeof(buf));
    assert(uvwasi_serdes_read_uint64_t(buf, sizeof(canary)) == 0xabcdef0123llu);
  }
}

void test_fdstat_t(void) {
  uvwasi_fdstat_t stat = {
    .fs_filetype = UVWASI_FILETYPE_DIRECTORY,
    .fs_flags = UVWASI_FDFLAG_APPEND,
    .fs_rights_base = UVWASI_RIGHT_FD_WRITE,
    .fs_rights_inheriting = UVWASI_RIGHT_FD_WRITE
  };

  char data[ADD_CANARIES(fdstat_t)] = { 0 };
  use_canaries(data, sizeof(data));
  uvwasi_serdes_write_fdstat_t(data, sizeof(canary), &stat);
  check_canaries(data, sizeof(data));
  /* TODO(tniessen): Check result of serialization. */

  uvwasi_fdstat_t deserialized;
  uvwasi_serdes_read_fdstat_t(data, sizeof(canary), &deserialized);
  assert(deserialized.fs_filetype == stat.fs_filetype);
  assert(deserialized.fs_flags == stat.fs_flags);
  assert(deserialized.fs_rights_base == stat.fs_rights_base);
  assert(deserialized.fs_rights_inheriting == stat.fs_rights_inheriting);
}

void test_filestat_t(void) {
  uvwasi_filestat_t stat = {
    .st_dev = 0x1234567812345678llu,
    .st_ino = 0x8765432187654321llu,
    .st_filetype = UVWASI_FILETYPE_REGULAR_FILE,
    .st_nlink = 0x1000000000000001llu,
    .st_size = 0x9999999999999999llu,
    .st_atim = 0x8888888888888888llu,
    .st_mtim = 0x7777777777777777llu,
    .st_ctim = 0x6666666666666666llu
  };

  char data[ADD_CANARIES(filestat_t)] = { 0 };
  use_canaries(data, sizeof(data));
  uvwasi_serdes_write_filestat_t(data, sizeof(canary), &stat);
  check_canaries(data, sizeof(data));
  /* TODO(tniessen): Check result of serialization. */

  uvwasi_filestat_t deserialized;
  uvwasi_serdes_read_filestat_t(data, sizeof(canary), &deserialized);
  assert(deserialized.st_dev == stat.st_dev);
  assert(deserialized.st_ino == stat.st_ino);
  assert(deserialized.st_filetype == stat.st_filetype);
  assert(deserialized.st_nlink == stat.st_nlink);
  assert(deserialized.st_size == stat.st_size);
  assert(deserialized.st_atim == stat.st_atim);
  assert(deserialized.st_mtim == stat.st_mtim);
  assert(deserialized.st_ctim == stat.st_ctim);
}

void test_prestat_t(void) {
  uvwasi_prestat_t stat = {
    .pr_type = UVWASI_PREOPENTYPE_DIR,
    .u = {
      .dir = {
        .pr_name_len = 100
      }
    }
  };

  char data[ADD_CANARIES(prestat_t)] = { 0 };
  use_canaries(data, sizeof(data));
  uvwasi_serdes_write_prestat_t(data, sizeof(canary), &stat);
  check_canaries(data, sizeof(data));
  /* TODO(tniessen): Check result of serialization. */

  uvwasi_prestat_t deserialized;
  uvwasi_serdes_read_prestat_t(data, sizeof(canary), &deserialized);
  assert(deserialized.pr_type == stat.pr_type);
  assert(deserialized.u.dir.pr_name_len == stat.u.dir.pr_name_len);
}

void test_event_t(void) {
  uvwasi_event_t event = {
    .userdata = 0xabcdabcdabcdabcdllu,
    .error = 0xabcd,
    .type = UVWASI_EVENTTYPE_CLOCK
  };

  char data[ADD_CANARIES(event_t)] = { 0 };
  use_canaries(data, sizeof(data));
  uvwasi_serdes_write_event_t(data, sizeof(canary), &event);
  check_canaries(data, sizeof(data));
  /* TODO(tniessen): Check result of serialization. */

  uvwasi_event_t deserialized = { 0 };
  uvwasi_serdes_read_event_t(data, sizeof(canary), &deserialized);
  assert(deserialized.userdata == event.userdata);
  assert(deserialized.error == event.error);
  assert(deserialized.type == event.type);
  assert(deserialized.u.fd_readwrite.nbytes == 0);
  assert(deserialized.u.fd_readwrite.flags == 0);

  event.type = UVWASI_EVENTTYPE_FD_READ;
  event.u.fd_readwrite.nbytes = 1000;
  event.u.fd_readwrite.flags = UVWASI_EVENT_FD_READWRITE_HANGUP;
  uvwasi_serdes_write_event_t(data, sizeof(canary), &event);
  /* TODO(tniessen): Check result of serialization. */

  memset(&deserialized, 0, sizeof(deserialized));
  uvwasi_serdes_read_event_t(data, sizeof(canary), &deserialized);
  assert(deserialized.userdata == event.userdata);
  assert(deserialized.error == event.error);
  assert(deserialized.type == event.type);
  assert(deserialized.u.fd_readwrite.nbytes == event.u.fd_readwrite.nbytes);
  assert(deserialized.u.fd_readwrite.flags == event.u.fd_readwrite.flags);

  event.type = UVWASI_EVENTTYPE_FD_WRITE;
  uvwasi_serdes_write_event_t(data, sizeof(canary), &event);
  /* TODO(tniessen): Check result of serialization. */

  memset(&deserialized, 0, sizeof(deserialized));
  uvwasi_serdes_read_event_t(data, sizeof(canary), &deserialized);
  assert(deserialized.userdata == event.userdata);
  assert(deserialized.error == event.error);
  assert(deserialized.type == event.type);
  assert(deserialized.u.fd_readwrite.nbytes == event.u.fd_readwrite.nbytes);
  assert(deserialized.u.fd_readwrite.flags == event.u.fd_readwrite.flags);
}

void test_subscription_t(void) {
  uvwasi_subscription_t subscription = {
    .userdata = 0xabcdabcdabcdabcdllu,
    .type = UVWASI_EVENTTYPE_CLOCK,
    .u = {
      .clock = {
        .clock_id = 0xabcdabcdu,
        .timeout = 0xabcdabcdabcdabcdllu,
        .precision = 0xdcbadcbadcbadcballu,
        .flags = UVWASI_SUBSCRIPTION_CLOCK_ABSTIME
      }
    }
  };

  char data[ADD_CANARIES(subscription_t)] = { 0 };
  use_canaries(data, sizeof(data));
  uvwasi_serdes_write_subscription_t(data, sizeof(canary), &subscription);
  check_canaries(data, sizeof(data));
  /* TODO(tniessen): Check result of serialization. */

  uvwasi_subscription_t deserialized = { 0 };
  uvwasi_serdes_read_subscription_t(data, sizeof(canary), &deserialized);
  assert(deserialized.userdata == subscription.userdata);
  assert(deserialized.type == subscription.type);
  assert(deserialized.u.clock.clock_id == subscription.u.clock.clock_id);
  assert(deserialized.u.clock.timeout == subscription.u.clock.timeout);
  assert(deserialized.u.clock.precision == subscription.u.clock.precision);
  assert(deserialized.u.clock.flags == subscription.u.clock.flags);

  subscription.type = UVWASI_EVENTTYPE_FD_READ;
  subscription.u.fd_readwrite.fd = 0xabcdabcdu;
  uvwasi_serdes_write_subscription_t(data, sizeof(canary), &subscription);
  /* TODO(tniessen): Check result of serialization. */

  memset(&deserialized, 0, sizeof(deserialized));
  uvwasi_serdes_read_subscription_t(data, sizeof(canary), &deserialized);
  assert(deserialized.userdata == subscription.userdata);
  assert(deserialized.type == subscription.type);
  assert(deserialized.u.fd_readwrite.fd == subscription.u.fd_readwrite.fd);

  subscription.type = UVWASI_EVENTTYPE_FD_WRITE;
  uvwasi_serdes_write_subscription_t(data, sizeof(canary), &subscription);
  /* TODO(tniessen): Check result of serialization. */

  memset(&deserialized, 0, sizeof(deserialized));
  uvwasi_serdes_read_subscription_t(data, sizeof(canary), &deserialized);
  assert(deserialized.userdata == subscription.userdata);
  assert(deserialized.type == subscription.type);
  assert(deserialized.u.fd_readwrite.fd == subscription.u.fd_readwrite.fd);
}
