#ifndef __UVWASI_SERDES_H__
#define __UVWASI_SERDES_H__

#include "wasi_types.h"

//
// Basic uint{8,16,32,64}_t read/write functions.
//

#define BASIC_TYPE(type)                                                       \
  void uvwasi_serdes_write_##type(void* ptr, size_t offset, type value);       \
  type uvwasi_serdes_read_##type(const void* ptr, size_t offset);              \

#define UVWASI_SERDES_SIZE_uint8_t sizeof(uint8_t)
BASIC_TYPE(uint8_t)
#define UVWASI_SERDES_SIZE_uint16_t sizeof(uint16_t)
BASIC_TYPE(uint16_t)
#define UVWASI_SERDES_SIZE_uint32_t sizeof(uint32_t)
BASIC_TYPE(uint32_t)
#define UVWASI_SERDES_SIZE_uint64_t sizeof(uint64_t)
BASIC_TYPE(uint64_t)

#undef BASIC_TYPE

//
// WASI structure read/write functions.
//

#define STRUCT(name) \
  void uvwasi_serdes_write_##name(void* ptr,                                   \
                                  size_t offset,                               \
                                  const uvwasi_##name* value);                 \
  void uvwasi_serdes_read_##name(const void* ptr,                              \
                                 size_t offset,                                \
                                 uvwasi_##name* value);                        \

#define UVWASI_SERDES_SIZE_fdstat_t 24
STRUCT(fdstat_t)

#define UVWASI_SERDES_SIZE_filestat_t 64
STRUCT(filestat_t)

#define UVWASI_SERDES_SIZE_prestat_t 8
STRUCT(prestat_t)

#define UVWASI_SERDES_SIZE_event_t 32
STRUCT(event_t)

#define UVWASI_SERDES_SIZE_subscription_t 48
STRUCT(subscription_t)

#undef STRUCT

//
// Helper macros for bound checking.
//

#define UVWASI_SERDES_CHECK_BOUNDS(offset, size, type)                         \
  ((offset) >= 0 &&                                                            \
   (size) > (offset) &&                                                        \
   (UVWASI_SERDES_SIZE_##type <= (size) - (offset)))                           \

#define UVWASI_SERDES_CHECK_ARRAY_BOUNDS(offset, size, type, count)            \
  ((offset) >= 0 &&                                                            \
   (size) > (offset) &&                                                        \
   (count) >= 0 &&                                                             \
   ((count) * UVWASI_SERDES_SIZE_##type <= (size) - (offset)))                 \

#endif /* __UVWASI_SERDES_H__ */
