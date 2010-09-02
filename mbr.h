#ifndef _MBR_H_
#define _MBR_H_

#include <types.h>

#define DOS_PART_EXT   0x05
#define DOS_PART_FAT16 0x06
#define DOS_PART_KAOS  0xca
#define DOS_PART_LINUX_SWAP  0x82
#define DOS_PART_LINUX 0x83

struct partition_entry {
  u_int8_t active;
  u_int8_t start_head;
  u_int16_t start_cylsec;
  u_int8_t type;
  u_int8_t end_head;
  u_int16_t end_cylsec;
  u_int32_t lba_sector;
  u_int32_t lba_len;
}; 
typedef struct partition_entry partition_entry_t;

struct mbr {
  u_int8_t code[446];
  partition_entry_t pe[4];
  u_int16_t signature;
} __attribute__((packed));
typedef struct mbr mbr_t;

#endif
