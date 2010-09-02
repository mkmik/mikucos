#ifndef __BYTEORDER_H__
#define __BYTEORDER_H__

#include <types.h>
#include <config.h>


static inline u_int16_t swap16(u_int16_t val) {
  u_int8_t *sin = (u_int8_t*)&val;
  u_int32_t out;
  u_int8_t *sout = (u_int8_t*)&out;
  
  sout[0] = sin[1];
  sout[1] = sin[0];

  return out;
}

static inline u_int32_t swap32(u_int32_t val) {
  u_int8_t *sin = (u_int8_t*)&val;
  u_int32_t out;
  u_int8_t *sout = (u_int8_t*)&out;
  
  sout[0] = sin[3];
  sout[1] = sin[2];
  sout[2] = sin[1];
  sout[3] = sin[0];

  return out;
}

#if ENDIANESS == LITTLE_ENDIAN

static inline u_int16_t le16_to_cpu(u_int16_t val) {
  return val;
}

static inline u_int32_t le32_to_cpu(u_int32_t val) {
  return val;
}

static inline u_int16_t cpu_to_le16(u_int16_t val) {
  return val;
}

static inline u_int32_t cpu_to_le32(u_int32_t val) {
  return val;
}

//

static inline u_int16_t be16_to_cpu(u_int16_t val) {
  return swap16(val);
}

static inline u_int32_t be32_to_cpu(u_int32_t val) {
  return swap32(val);
}

static inline u_int16_t cpu_to_be16(u_int16_t val) {
  return swap16(val);
}

static inline u_int32_t cpu_to_be32(u_int32_t val) {
  return swap32(val);
}

#elif ENDIANESS == BIG_ENDIAN
# error NOT IMPLEMENTED
#else
# error UNKNOWN ENDIANESS
#endif

static inline u_int16_t cpu_to_net16(u_int16_t val) {
  return cpu_to_be16(val);
}

static inline u_int32_t cpu_to_net32(u_int32_t val) {
  return cpu_to_be32(val);
}

static inline u_int16_t net16_to_cpu(u_int16_t val) {
  return cpu_to_be16(val);
}

static inline u_int32_t net32_to_cpu(u_int32_t val) {
  return cpu_to_be32(val);
}

#endif
