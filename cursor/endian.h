#ifndef __ENDIAN_H__
#define __ENDIAN_H__

#include <xcb/xcb.h>

#ifndef __LITTLE_ENDIAN__
 #if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
  #define __LITTLE_ENDIAN__
 #else
  #define __BIG_ENDIAN__
 #endif
#endif

uint16_t htobe16(uint16_t host_16bits);
uint16_t htole16(uint16_t host_16bits);
uint16_t be16toh(uint16_t big_endian_16bits);
uint16_t le16toh(uint16_t little_endian_16bits);

uint32_t htobe32(uint32_t host_32bits);
uint32_t htole32(uint32_t host_32bits);
uint32_t be32toh(uint32_t big_endian_32bits);
uint32_t le32toh(uint32_t little_endian_32bits);

uint64_t htobe64(uint64_t host_64bits);
uint64_t htole64(uint64_t host_64bits);
uint64_t be64toh(uint64_t big_endian_64bits);
uint64_t le64toh(uint64_t little_endian_64bits);

#endif /* __ENDIAN_H__ */