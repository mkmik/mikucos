#ifndef _PARTITION_H_
#define _PARTITION_H_

#include <device.h>

struct partition_private {
  device_t* disk;
  u_int64_t start;
  u_int64_t len;

  int number; // mainly for debugging
};
typedef struct partition_private partition_private_t;

// ops

/** returns the compound name of the partition.
 * A child of the containing device. 
 * for example: IDE/0/part2
 **/
char* partition_devfs_filename(device_t* this);
int partition_read_blocks(device_t* this, block_t* buffer, off_t start, size_t len);
int partition_get_device_option(device_t* this, const char* name, va_list args);

// api

/** called to scan for partitions in 'device' and 
 * to register them as childs of the driver 'this' */
void partition_lookup(driver_t* this, device_t *device);

#endif
