#ifndef __TYPES_H__
#define __TYPES_H__

/** signed 8 bit integer (byte) */
typedef char  int8_t;
/** signed 16 bit integer (word) */
typedef short int16_t;
/** signed 16 bit integer (longword, doubleword) */
typedef int   int32_t;
/** signed 64 bit integer (quadword, gcc extension) */
typedef long long int64_t;

/** unsigned 8 bit integer (byte) */
typedef unsigned char  u_int8_t;
/** unsigned 16 bit integer (word) */
typedef unsigned short u_int16_t;
/** unsigned 32 bit integer (longword, doubleword) */
typedef unsigned int   u_int32_t;
/** unsigned 64 bit integer (quadword, gcc extension) */
typedef unsigned long long u_int64_t;

/** generic size of something */
typedef unsigned int size_t;
/** generic size of something */
typedef int ssize_t;
/** generic offset of something */
typedef unsigned int off_t;

// needed by vmalloc
#define _typ_ssize_t 1

// needed by acaros code
#define PACKED __attribute__((packed))
typedef u_int32_t pointer_t;

#endif
