#ifndef __UVWASI_UV_MAPPING_H__
#define __UVWASI_UV_MAPPING_H__

#include "uv.h"
#include "wasi_types.h"

#define NANOS_PER_SEC 1000000000

uvwasi_errno_t uvwasi__translate_uv_error(int err);
int uvwasi__translate_to_uv_signal(uvwasi_signal_t sig);
uvwasi_timestamp_t uvwasi__timespec_to_timestamp(const uv_timespec_t* ts);

#endif /* __UVWASI_UV_MAPPING_H__ */
