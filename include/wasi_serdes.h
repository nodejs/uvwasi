#ifndef __UVWASI_SERDES_H__
#define __UVWASI_SERDES_H__

#include "wasi_types.h"

/* Basic uint{8,16,32,64}_t read/write functions. */

#define BASIC_TYPE_(name, type)                                               \
  void uvwasi_serdes_write_##name(void* ptr, size_t offset, type value);      \
  type uvwasi_serdes_read_##name(const void* ptr, size_t offset);             \

#define BASIC_TYPE(type) BASIC_TYPE_(type, type)
#define BASIC_TYPE_UVWASI(type) BASIC_TYPE_(type, uvwasi_##type)

#define UVWASI_SERDES_SIZE_uint8_t sizeof(uint8_t)
BASIC_TYPE(uint8_t)
#define UVWASI_SERDES_SIZE_uint16_t sizeof(uint16_t)
BASIC_TYPE(uint16_t)
#define UVWASI_SERDES_SIZE_uint32_t sizeof(uint32_t)
BASIC_TYPE(uint32_t)
#define UVWASI_SERDES_SIZE_uint64_t sizeof(uint64_t)
BASIC_TYPE(uint64_t)

#define UVWASI_SERDES_SIZE_advice_t sizeof(uvwasi_advice_t)
BASIC_TYPE_UVWASI(advice_t)
#define UVWASI_SERDES_SIZE_clockid_t sizeof(uvwasi_clockid_t)
BASIC_TYPE_UVWASI(clockid_t)
#define UVWASI_SERDES_SIZE_device_t sizeof(uvwasi_device_t)
BASIC_TYPE_UVWASI(device_t)
#define UVWASI_SERDES_SIZE_dircookie_t sizeof(uvwasi_dircookie_t)
BASIC_TYPE_UVWASI(dircookie_t)
#define UVWASI_SERDES_SIZE_eventrwflags_t sizeof(uvwasi_eventrwflags_t)
BASIC_TYPE_UVWASI(eventrwflags_t)
#define UVWASI_SERDES_SIZE_eventtype_t sizeof(uvwasi_eventtype_t)
BASIC_TYPE_UVWASI(eventtype_t)
#define UVWASI_SERDES_SIZE_exitcode_t sizeof(uvwasi_exitcode_t)
BASIC_TYPE_UVWASI(exitcode_t)
#define UVWASI_SERDES_SIZE_fd_t sizeof(uvwasi_fd_t)
BASIC_TYPE_UVWASI(fd_t)
#define UVWASI_SERDES_SIZE_fdflags_t sizeof(uvwasi_fdflags_t)
BASIC_TYPE_UVWASI(fdflags_t)
#define UVWASI_SERDES_SIZE_filesize_t sizeof(uvwasi_filesize_t)
BASIC_TYPE_UVWASI(filesize_t)
#define UVWASI_SERDES_SIZE_fstflags_t sizeof(uvwasi_fstflags_t)
BASIC_TYPE_UVWASI(fstflags_t)
#define UVWASI_SERDES_SIZE_inode_t sizeof(uvwasi_inode_t)
BASIC_TYPE_UVWASI(inode_t)
#define UVWASI_SERDES_SIZE_linkcount_t sizeof(uvwasi_linkcount_t)
BASIC_TYPE_UVWASI(linkcount_t)
#define UVWASI_SERDES_SIZE_lookupflags_t sizeof(uvwasi_lookupflags_t)
BASIC_TYPE_UVWASI(lookupflags_t)
#define UVWASI_SERDES_SIZE_oflags_t sizeof(uvwasi_oflags_t)
BASIC_TYPE_UVWASI(oflags_t)
#define UVWASI_SERDES_SIZE_preopentype_t sizeof(uvwasi_preopentype_t)
BASIC_TYPE_UVWASI(preopentype_t)
#define UVWASI_SERDES_SIZE_riflags_t sizeof(uvwasi_riflags_t)
BASIC_TYPE_UVWASI(riflags_t)
#define UVWASI_SERDES_SIZE_rights_t sizeof(uvwasi_rights_t)
BASIC_TYPE_UVWASI(rights_t)
#define UVWASI_SERDES_SIZE_roflags_t sizeof(uvwasi_roflags_t)
BASIC_TYPE_UVWASI(roflags_t)
#define UVWASI_SERDES_SIZE_sdflags_t sizeof(uvwasi_sdflags_t)
BASIC_TYPE_UVWASI(sdflags_t)
#define UVWASI_SERDES_SIZE_siflags_t sizeof(uvwasi_siflags_t)
BASIC_TYPE_UVWASI(siflags_t)
#define UVWASI_SERDES_SIZE_inode_t sizeof(uvwasi_inode_t)
BASIC_TYPE_UVWASI(inode_t)
#define UVWASIS_SERDES_SIZE_signal_t sizeof(uvwasi_signal_t)
BASIC_TYPE_UVWASI(signal_t)
#define UVWASIS_SERDES_SIZE_subclockflags_t sizeof(uvwasi_subclockflags_t)
BASIC_TYPE_UVWASI(subclockflags_t)
#define UVWASIS_SERDES_SIZE_timestamp_t sizeof(uvwasi_timestamp_t)
BASIC_TYPE_UVWASI(timestamp_t)
#define UVWASIS_SERDES_SIZE_userdata_t sizeof(uvwasi_userdata_t)
BASIC_TYPE_UVWASI(userdata_t)
#define UVWASIS_SERDES_SIZE_whence_t sizeof(uvwasi_whence_t)
BASIC_TYPE_UVWASI(whence_t)

#undef BASIC_TYPE_UVWASI
#undef BASIC_TYPE
#undef BASIC_TYPE_

/* WASI structure read/write functions. */

#define STRUCT(name) \
  void uvwasi_serdes_write_##name(void* ptr,                                  \
                                  size_t offset,                              \
                                  const uvwasi_##name* value);                \
  void uvwasi_serdes_read_##name(const void* ptr,                             \
                                 size_t offset,                               \
                                 uvwasi_##name* value);                       \

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

/* Helper macros for bound checking. */

#define UVWASI_SERDES_CHECK_BOUNDS(offset, end, type)                         \
  ((offset) >= 0 &&                                                           \
   (end) > (offset) &&                                                        \
   (UVWASI_SERDES_SIZE_##type <= (end) - (offset)))                           \

#define UVWASI_SERDES_CHECK_ARRAY_BOUNDS(offset, end, type, count)            \
  ((offset) >= 0 &&                                                           \
   (end) > (offset) &&                                                        \
   (count) >= 0 &&                                                            \
   ((count) * UVWASI_SERDES_SIZE_##type) / UVWASI_SERDES_SIZE_##type ==       \
    (count) &&                                                                \
   ((count) * UVWASI_SERDES_SIZE_##type <= (end) - (offset)))                 \

#endif /* __UVWASI_SERDES_H__ */
