#ifndef _FAT_LOW_H_
#define _FAT_LOW_H_

#include <types.h>

struct physical_mounted_filesystem;

struct fat_common_bootsector {
  u_int8_t jmp[3];
  char OEM[8];
  u_int16_t bytes_per_sector;
  u_int8_t sectors_per_cluster;
  u_int16_t reserved_sector;
  u_int8_t number_of_fats;
  u_int16_t root_dir_entries;
  u_int16_t logical_sectors;
  u_int8_t medium_descriptor;
  u_int16_t sectors_per_fat;
  u_int16_t sectors_per_track;
  u_int16_t number_heads;
} PACKED;

typedef struct fat_common_bootsector fat_common_bootsector_t;

struct fat16_bootsector {
  //  fat_common_bootsector_t;
  u_int8_t jmp[3];
  char OEM[8];
  u_int16_t bytes_per_sector;
  u_int8_t sectors_per_cluster;
  u_int16_t reserved_sector;
  u_int8_t number_of_fats;
  u_int16_t root_dir_entries;
  u_int16_t logical_sectors;
  u_int8_t medium_descriptor;
  u_int16_t sectors_per_fat;
  u_int16_t sectors_per_track;
  u_int16_t number_heads;
  // end common bootsector
  u_int32_t number_hidden_sectors;
  u_int32_t big_logical_sectors; 
  u_int8_t logical_drive_number;
  u_int8_t reserved;
  u_int8_t extended_signature;
  u_int32_t serial_number;
  char label[11];
  char type[8];
} PACKED;

typedef struct fat16_bootsector fat16_bootsector_t;

struct fat32_bootsector {
  //fat_common_bootsector_t;
  u_int8_t jmp[3];
  char OEM[8];
  u_int16_t bytes_per_sector;
  u_int8_t sectors_per_cluster;
  u_int16_t reserved_sector;
  u_int8_t number_of_fats;
  u_int16_t root_dir_entries;
  u_int16_t logical_sectors;
  u_int8_t medium_descriptor;
  u_int16_t sectors_per_fat;
  u_int16_t sectors_per_track;
  u_int16_t number_heads;
  // end common bootsector
  u_int32_t number_hidden_sectors;
  u_int32_t big_logical_sectors; 
  u_int32_t big_sectors_per_fat_sectors; 
  u_int16_t mirror_flags;
  u_int32_t root_start;
  u_int16_t information_start;
  u_int16_t backup_bootsector;
  u_int8_t reserved2[12];
  u_int8_t logical_drive_number;
  u_int8_t reserved;
  u_int8_t extended_signature;
  u_int32_t serial_number;
  char label[11];
  char type[8];
} PACKED;

typedef struct fat32_bootsector fat32_bootsector_t;

struct fat_dir_entry {
  char name[8];
  char ext[3];
  union {
    u_int8_t attribute;
    struct {
      u_int8_t read_only:1;
      u_int8_t hidden:1;
      u_int8_t system_file:1;
      u_int8_t volume_label:1;
      u_int8_t subdirectory:1;
      u_int8_t archive:1;
      u_int8_t _unused:2;
    };
  };
  u_int8_t lcase;
  u_int8_t ctime_ms;
  u_int16_t ctime;
  u_int16_t cdate;
  u_int16_t adate;
  u_int8_t reserved[2];
  u_int16_t time;
  u_int16_t date;
  u_int16_t first_cluster;
  /** in bytes */
  u_int32_t length;
} PACKED; 

struct vfat_slot {          // Up to 13 characters of a long name
  u_int8_t id;              // sequence number for slot
  u_int16_t name0_4[5];     // first 5 characters in name
  u_int8_t attr;            // attribute byte
  u_int8_t reserved;        // always 0
  u_int8_t alias_checksum;  // checksum for 8.3 alias
  u_int16_t name5_10[6];    // 6 more characters in name
  u_int16_t start;          // starting cluster number
  u_int16_t name11_12[2];   // last 2 characters in name
} PACKED;

typedef struct fat_dir_entry fat_dir_entry_t;

/** returns the contents of the fat for 'cluster' */
extern u_int16_t fat16_read_entry(struct physical_mounted_filesystem* this, off_t cluster);

extern void fat_read_page(struct physical_mounted_filesystem* this, int page);

static inline u_int16_t fat16_eof() {
  return 0xFFFF;
}

static inline int fat16_is_eof(u_int16_t entry) {
  return entry >= 0xFFF8;
}

static inline int fat16_is_free(u_int16_t entry) {
  return entry == 0;
}

#endif

