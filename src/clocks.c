#include "uv.h"
#include "clocks.h"
#include "wasi_types.h"
#include "uv_mapping.h"


uvwasi_errno_t uvwasi__clock_gettime_realtime(uvwasi_timestamp_t* time) {
  uv_timeval64_t tv;
  int r;

  r = uv_gettimeofday(&tv);
  if (r != 0)
    return uvwasi__translate_uv_error(r);

  *time = (tv.tv_sec * NANOS_PER_SEC) + (tv.tv_usec * 1000);
  return UVWASI_ESUCCESS;
}
