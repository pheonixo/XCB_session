#include "endian.h"

uint16_t
htobe16(uint16_t a) {
 #ifdef __LITTLE_ENDIAN__
  return (  ((a & (uint16_t)0xFF00) >> 8)
          | ((a & (uint16_t)0x00FF) << 8) );
 #else
  return a;
 #endif
}

uint16_t
htole16(uint16_t a) {
 #ifdef __LITTLE_ENDIAN__
  return a;
 #else
  return (  ((a & (uint16_t)0xFF00) >> 8)
          | ((a & (uint16_t)0x00FF) << 8) );
 #endif
}

uint16_t
be16toh(uint16_t a) {
 #ifdef __LITTLE_ENDIAN__
  return (  ((a & (uint16_t)0x0FF00) >> 8)
          | ((a & (uint16_t)0x000FF) << 8) );
 #else
  return a;
 #endif
}

uint16_t
le16toh(uint16_t a) {
 #ifdef __LITTLE_ENDIAN__
  return a;
 #else
  return (  ((a & (uint16_t)0xFF00) >> 8)
          | ((a & (uint16_t)0x00FF) << 8) );
 #endif
}

uint32_t
htobe32(uint32_t a) {
 #ifdef __LITTLE_ENDIAN__
  return (  ((a & (uint32_t)0xFF000000) >> 24)
          | ((a & (uint32_t)0x00FF0000) >> 8)
          | ((a & (uint32_t)0x0000FF00) << 8)
          | ((a & (uint32_t)0x000000FF) << 24) );
 #else
  return a;
 #endif
}

uint32_t
htole32(uint32_t a) {
 #ifdef __LITTLE_ENDIAN__
  return a;
 #else
  return (  ((a & (uint32_t)0xFF000000) >> 24)
          | ((a & (uint32_t)0x00FF0000) >> 8)
          | ((a & (uint32_t)0x0000FF00) << 8)
          | ((a & (uint32_t)0x000000FF) << 24) );
#endif
}

uint32_t
be32toh(uint32_t a) {
 #ifdef __LITTLE_ENDIAN__
  return (  ((a & (uint32_t)0xFF000000) >> 24)
          | ((a & (uint32_t)0x00FF0000) >> 8)
          | ((a & (uint32_t)0x0000FF00) << 8)
          | ((a & (uint32_t)0x000000FF) << 24) );
 #else
  return a;
 #endif
}

uint32_t
le32toh(uint32_t a) {
 #ifdef __LITTLE_ENDIAN__
  return a;
 #else
  return (  ((a & (uint32_t)0xFF000000) >> 24)
          | ((a & (uint32_t)0x00FF0000) >> 8)
          | ((a & (uint32_t)0x0000FF00) << 8)
          | ((a & (uint32_t)0x000000FF) << 24) );
#endif
}

uint64_t
htobe64(uint64_t a) {
 #ifdef __LITTLE_ENDIAN__
  return (  ((a & (uint64_t)0xFF00000000000000) >> 56)
          | ((a & (uint64_t)0x00FF000000000000) >> 40)
          | ((a & (uint64_t)0x0000FF0000000000) >> 24)
          | ((a & (uint64_t)0x000000FF00000000) >> 8)
          | ((a & (uint64_t)0x00000000FF000000) << 8)
          | ((a & (uint64_t)0x0000000000FF0000) << 24)
          | ((a & (uint64_t)0x000000000000FF00) << 40)
          | ((a & (uint64_t)0x00000000000000FF) << 56) );
 #else
  return a;
 #endif
}

uint64_t
htole64(uint64_t a) {
 #ifdef __LITTLE_ENDIAN__
  return a;
 #else
  return (  ((a & (uint64_t)0xFF00000000000000) >> 56)
          | ((a & (uint64_t)0x00FF000000000000) >> 40)
          | ((a & (uint64_t)0x0000FF0000000000) >> 24)
          | ((a & (uint64_t)0x000000FF00000000) >> 8)
          | ((a & (uint64_t)0x00000000FF000000) << 8)
          | ((a & (uint64_t)0x0000000000FF0000) << 24)
          | ((a & (uint64_t)0x000000000000FF00) << 40)
          | ((a & (uint64_t)0x00000000000000FF) << 56) );
#endif
}

uint64_t
be64toh(uint64_t a) {
 #ifdef __LITTLE_ENDIAN__
  return (  ((a & (uint64_t)0xFF00000000000000) >> 56)
          | ((a & (uint64_t)0x00FF000000000000) >> 40)
          | ((a & (uint64_t)0x0000FF0000000000) >> 24)
          | ((a & (uint64_t)0x000000FF00000000) >> 8)
          | ((a & (uint64_t)0x00000000FF000000) << 8)
          | ((a & (uint64_t)0x0000000000FF0000) << 24)
          | ((a & (uint64_t)0x000000000000FF00) << 40)
          | ((a & (uint64_t)0x00000000000000FF) << 56) );
 #else
  return a;
 #endif
}

uint64_t
le64toh(uint64_t a) {
 #ifdef __LITTLE_ENDIAN__
  return a;
 #else
  return (  ((a & (uint64_t)0xFF00000000000000) >> 56)
          | ((a & (uint64_t)0x00FF000000000000) >> 40)
          | ((a & (uint64_t)0x0000FF0000000000) >> 24)
          | ((a & (uint64_t)0x000000FF00000000) >> 8)
          | ((a & (uint64_t)0x00000000FF000000) << 8)
          | ((a & (uint64_t)0x0000000000FF0000) << 24)
          | ((a & (uint64_t)0x000000000000FF00) << 40)
          | ((a & (uint64_t)0x00000000000000FF) << 56) );
#endif
}

