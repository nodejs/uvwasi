#include "uvwasi.h"

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

#define TYPE_SWITCH switch(value->type)

#define ALL_STRUCTS(STRUCT, FIELD)                                             \
                                                                               \
  STRUCT(fdstat_t) {                                                           \
    FIELD( 0, uint8_t,  fs_filetype);                                          \
    FIELD( 2, uint16_t, fs_flags);                                             \
    FIELD( 8, uint64_t, fs_rights_base);                                       \
    FIELD(16, uint64_t, fs_rights_inheriting);                                 \
  }                                                                            \
                                                                               \
  STRUCT(filestat_t) {                                                         \
    FIELD( 0, uint64_t, st_dev);                                               \
    FIELD( 8, uint64_t, st_ino);                                               \
    FIELD(16, uint8_t,  st_filetype);                                          \
    FIELD(24, uint64_t, st_nlink);                                             \
    FIELD(32, uint64_t, st_size);                                              \
    FIELD(40, uint64_t, st_atim);                                              \
    FIELD(48, uint64_t, st_mtim);                                              \
    FIELD(56, uint64_t, st_ctim);                                              \
  }                                                                            \
                                                                               \
  STRUCT(prestat_t) {                                                          \
    FIELD(0, uint32_t, pr_type);                                               \
    FIELD(4, uint32_t, u.dir.pr_name_len);                                     \
  }                                                                            \
                                                                               \
  STRUCT(event_t) {                                                            \
    FIELD( 0, uint64_t, userdata);                                             \
    FIELD( 8, uint16_t, error);                                                \
    FIELD(10, uint8_t,  type);                                                 \
    TYPE_SWITCH {                                                              \
      case UVWASI_EVENTTYPE_FD_READ:                                           \
      case UVWASI_EVENTTYPE_FD_WRITE:                                          \
        FIELD(16, uint64_t, u.fd_readwrite.nbytes);                            \
        FIELD(24, uint16_t, u.fd_readwrite.flags);                             \
    }                                                                          \
  }                                                                            \
                                                                               \
  STRUCT(subscription_t) {                                                     \
    FIELD(0, uint64_t, userdata);                                              \
    FIELD(8, uint8_t,  type);                                                  \
    TYPE_SWITCH {                                                              \
      case UVWASI_EVENTTYPE_CLOCK:                                             \
        FIELD(16, uint32_t, u.clock.clock_id);                                 \
        FIELD(24, uint64_t, u.clock.timeout);                                  \
        FIELD(32, uint64_t, u.clock.precision);                                \
        FIELD(40, uint16_t, u.clock.flags);                                    \
        break;                                                                 \
      case UVWASI_EVENTTYPE_FD_READ:                                           \
      case UVWASI_EVENTTYPE_FD_WRITE:                                          \
        FIELD(16, uint32_t, u.fd_readwrite.fd);                                \
    }                                                                          \
  }                                                                            \

#define WRITE_STRUCT(name)                                                     \
  void uvwasi_serdes_write_##name(void* ptr,                                   \
                                  size_t offset,                               \
                                  const uvwasi_##name* value)                  \

#define READ_STRUCT(name)                                                      \
  void uvwasi_serdes_read_##name(const void* ptr,                              \
                                 size_t offset,                                \
                                 uvwasi_##name* value)                         \

#define WRITE_FIELD(field_offset, type, field)                                 \
  do {                                                                         \
    uvwasi_serdes_write_##type(ptr, offset + field_offset, value->field);      \
  } while (0)                                                                  \

#define READ_FIELD(field_offset, type, field)                                  \
  do {                                                                         \
    value->field = uvwasi_serdes_read_##type(ptr, offset + field_offset);      \
  } while (0)                                                                  \

ALL_STRUCTS(WRITE_STRUCT, WRITE_FIELD)
ALL_STRUCTS(READ_STRUCT, READ_FIELD)
