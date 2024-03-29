#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "uvwasi.h"
#include "test-common.h"

int main(void) {
  uvwasi_t uvwasi;
  uvwasi_options_t init_options;
  uvwasi_errno_t err;
  uvwasi_timestamp_t time;
  uvwasi_timestamp_t precision = 1000;

  setup_test_environment();

  uvwasi_options_init(&init_options);

  err = uvwasi_init(&uvwasi, &init_options);
  assert(err == 0);

  err = uvwasi_clock_time_get(&uvwasi, UVWASI_CLOCK_REALTIME, precision, &time);
  assert(err == 0);
  assert(time > 0);

  err = uvwasi_clock_time_get(&uvwasi, UVWASI_CLOCK_MONOTONIC, precision, &time);
  assert(err == 0);
  assert(time > 0);

  // use some cpu time
  int count = 0;
  for (int i = 0; i < 100000000; i++) {
    count++;
  }
  assert(count == 100000000);

  err = uvwasi_clock_time_get(&uvwasi, UVWASI_CLOCK_PROCESS_CPUTIME_ID, precision, &time);
  assert(err == 0);
  assert(time > 0);

  err = uvwasi_clock_time_get(&uvwasi, UVWASI_CLOCK_THREAD_CPUTIME_ID, precision, &time);
  assert(err == 0);
  assert(time > 0);

  uvwasi_destroy(&uvwasi);

  return 0;
}
