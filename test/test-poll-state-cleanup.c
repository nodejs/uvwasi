#include <assert.h>
#include "uvwasi.h"

int main(void) {
  uvwasi_t uvwasi;
  uvwasi_options_t init_options;
  uvwasi_subscription_t sub;
  uvwasi_event_t event;
  uvwasi_size_t nevents;
  uvwasi_errno_t err;

  uvwasi_options_init(&init_options);
  err = uvwasi_init(&uvwasi, &init_options);
  assert(err == 0);

  sub.userdata = 42;
  sub.u.tag = UVWASI_EVENTTYPE_CLOCK;
  sub.u.u.clock.clock_id = UVWASI_CLOCK_REALTIME;
  sub.u.u.clock.timeout = 0;
  sub.u.u.clock.precision = 1;
  sub.u.u.clock.flags = 0;

  err = uvwasi_poll_oneoff(&uvwasi, &sub, &event, 1, &nevents);
  assert(err == 0);
  assert(nevents == 1);
  assert(event.userdata == 42);
  assert(event.error == UVWASI_ESUCCESS);
  assert(event.type == UVWASI_EVENTTYPE_CLOCK);

  uvwasi_destroy(&uvwasi);
  return 0;
}
