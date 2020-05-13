#include "wasi_serdes.h"
#include "wasi_types.h"

inline void uvwasi_serdes_write_uint64_t(void* ptr,
                                         size_t offset,
                                         uint64_t value) {
  uvwasi_serdes_write_uint32_t(ptr, offset, (uint32_t) value);
  uvwasi_serdes_write_uint32_t(ptr, offset + 4, value >> 32);
}

inline void uvwasi_serdes_write_uint32_t(void* ptr,
                                         size_t offset,
                                         uint32_t value) {
  uvwasi_serdes_write_uint16_t(ptr, offset, (uint16_t) value);
  uvwasi_serdes_write_uint16_t(ptr, offset + 2, value >> 16);
}

inline void uvwasi_serdes_write_uint16_t(void* ptr,
                                         size_t offset,
                                         uint16_t value) {
  uvwasi_serdes_write_uint8_t(ptr, offset, (uint8_t) value);
  uvwasi_serdes_write_uint8_t(ptr, offset + 1, value >> 8);
}

inline void uvwasi_serdes_write_uint8_t(void* ptr,
                                        size_t offset,
                                        uint8_t value) {
  ((uint8_t*) ptr)[offset] = value;
}

inline uint64_t uvwasi_serdes_read_uint64_t(const void* ptr, size_t offset) {
  uint64_t low = uvwasi_serdes_read_uint32_t(ptr, offset);
  uint64_t high = uvwasi_serdes_read_uint32_t(ptr, offset + 4);
  return low | (high << 32);
}

inline uint32_t uvwasi_serdes_read_uint32_t(const void* ptr, size_t offset) {
  uint32_t low = uvwasi_serdes_read_uint16_t(ptr, offset);
  uint32_t high = uvwasi_serdes_read_uint16_t(ptr, offset + 2);
  return low | (high << 16);
}

inline uint16_t uvwasi_serdes_read_uint16_t(const void* ptr, size_t offset) {
  uint16_t low = uvwasi_serdes_read_uint8_t(ptr, offset);
  uint16_t high = uvwasi_serdes_read_uint8_t(ptr, offset + 1);
  return low | (high << 8);
}

inline uint8_t uvwasi_serdes_read_uint8_t(const void* ptr,  size_t offset) {
  return ((const uint8_t*) ptr)[offset];
}

#define TYPE_SWITCH switch (value->type)

#define ALL_TYPES(STRUCT, FIELD, ALIAS)                                       \
                                                                              \
  ALIAS(advice_t,        uint8_t)                                             \
  ALIAS(clockid_t,       uint32_t)                                            \
  ALIAS(device_t,        uint64_t)                                            \
  ALIAS(dircookie_t,     uint64_t)                                            \
  ALIAS(errno_t,         uint16_t)                                            \
  ALIAS(eventrwflags_t,  uint16_t)                                            \
  ALIAS(eventtype_t,     uint8_t)                                             \
  ALIAS(exitcode_t,      uint32_t)                                            \
  ALIAS(fd_t,            uint32_t)                                            \
  ALIAS(fdflags_t,       uint16_t)                                            \
  ALIAS(filesize_t,      uint64_t)                                            \
  ALIAS(filetype_t,      uint8_t)                                             \
  ALIAS(fstflags_t,      uint16_t)                                            \
  ALIAS(inode_t,         uint64_t)                                            \
  ALIAS(linkcount_t,     uint64_t)                                            \
  ALIAS(lookupflags_t,   uint32_t)                                            \
  ALIAS(oflags_t,        uint16_t)                                            \
  ALIAS(preopentype_t,   uint8_t)                                             \
  ALIAS(riflags_t,       uint16_t)                                            \
  ALIAS(rights_t,        uint64_t)                                            \
  ALIAS(roflags_t,       uint16_t)                                            \
  ALIAS(sdflags_t,       uint8_t)                                             \
  ALIAS(siflags_t,       uint16_t)                                            \
  ALIAS(signal_t,        uint8_t)                                             \
  ALIAS(subclockflags_t, uint16_t)                                            \
  ALIAS(timestamp_t,     uint64_t)                                            \
  ALIAS(userdata_t,      uint64_t)                                            \
  ALIAS(whence_t,        uint8_t)                                             \
                                                                              \
  STRUCT(fdstat_t) {                                                          \
    FIELD( 0, filetype_t, fs_filetype);                                       \
    FIELD( 2, fdflags_t,  fs_flags);                                          \
    FIELD( 8, rights_t,   fs_rights_base);                                    \
    FIELD(16, rights_t,   fs_rights_inheriting);                              \
  }                                                                           \
                                                                              \
  STRUCT(filestat_t) {                                                        \
    FIELD( 0, device_t,    st_dev);                                           \
    FIELD( 8, inode_t,     st_ino);                                           \
    FIELD(16, filetype_t,  st_filetype);                                      \
    FIELD(24, linkcount_t, st_nlink);                                         \
    FIELD(32, filesize_t,  st_size);                                          \
    FIELD(40, timestamp_t, st_atim);                                          \
    FIELD(48, timestamp_t, st_mtim);                                          \
    FIELD(56, timestamp_t, st_ctim);                                          \
  }                                                                           \
                                                                              \
  STRUCT(prestat_t) {                                                         \
    FIELD(0, preopentype_t, pr_type);                                         \
    FIELD(4, uint32_t,      u.dir.pr_name_len);                               \
  }                                                                           \
                                                                              \
  STRUCT(event_t) {                                                           \
    FIELD( 0, userdata_t,  userdata);                                         \
    FIELD( 8, errno_t,     error);                                            \
    FIELD(10, eventtype_t, type);                                             \
    TYPE_SWITCH {                                                             \
      case UVWASI_EVENTTYPE_FD_READ:                                          \
      case UVWASI_EVENTTYPE_FD_WRITE:                                         \
        FIELD(16, filesize_t,     u.fd_readwrite.nbytes);                     \
        FIELD(24, eventrwflags_t, u.fd_readwrite.flags);                      \
    }                                                                         \
  }                                                                           \
                                                                              \
  STRUCT(subscription_t) {                                                    \
    FIELD(0, userdata_t,  userdata);                                          \
    FIELD(8, eventtype_t, type);                                              \
    TYPE_SWITCH {                                                             \
      case UVWASI_EVENTTYPE_CLOCK:                                            \
        FIELD(16, clockid_t,       u.clock.clock_id);                         \
        FIELD(24, timestamp_t,     u.clock.timeout);                          \
        FIELD(32, timestamp_t,     u.clock.precision);                        \
        FIELD(40, subclockflags_t, u.clock.flags);                            \
        break;                                                                \
      case UVWASI_EVENTTYPE_FD_READ:                                          \
      case UVWASI_EVENTTYPE_FD_WRITE:                                         \
        FIELD(16, fd_t, u.fd_readwrite.fd);                                   \
    }                                                                         \
  }                                                                           \

#define WRITE_STRUCT(name)                                                    \
  void uvwasi_serdes_write_##name(void* ptr,                                  \
                                  size_t offset,                              \
                                  const uvwasi_##name* value)                 \

#define READ_STRUCT(name)                                                     \
  void uvwasi_serdes_read_##name(const void* ptr,                             \
                                 size_t offset,                               \
                                 uvwasi_##name* value)                        \

#define WRITE_FIELD(field_offset, type, field)                                \
  do {                                                                        \
    uvwasi_serdes_write_##type(ptr, offset + field_offset, value->field);     \
  } while (0)                                                                 \

#define READ_FIELD(field_offset, type, field)                                 \
  do {                                                                        \
    value->field = uvwasi_serdes_read_##type(ptr, offset + field_offset);     \
  } while (0)                                                                 \

#define WRITE_ALIAS(new_name, old_name)                                       \
  void uvwasi_serdes_write_##new_name(void* ptr,                              \
                                      size_t offset,                          \
                                      uvwasi_##new_name value) {              \
    uvwasi_serdes_write_##old_name(ptr, offset, value);                       \
  }                                                                           \

#define READ_ALIAS(new_name, old_name)                                        \
  uvwasi_##new_name uvwasi_serdes_read_##new_name(const void* ptr,            \
                                                  size_t offset) {            \
    return uvwasi_serdes_read_##old_name(ptr, offset);                        \
  }                                                                           \

ALL_TYPES(WRITE_STRUCT, WRITE_FIELD, WRITE_ALIAS)
ALL_TYPES(READ_STRUCT, READ_FIELD, READ_ALIAS)
