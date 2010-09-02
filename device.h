#ifndef __DEVICE_H_
#define __DEVICE_H_

#include <types.h>
#include <list.h>
#include <avl.h>
#include <libc.h>
#include <stdarg.h>

#include <block.h>
#include <driver.h>
#include <invoke.h>

struct device_ops {
  void (*init)(device_t* this);
  char* (*devfs_filename)(device_t* this);
  void (*register_devfs)(device_t* this);
  /** reads "len" bytes from stream oriented device "this" at position "off" 
   * to buffer "buffer" 
   * and then updates the "off" variable to the next position.
   * returns the number of readed bytes or -1 when end of file is reached */
  ssize_t (*read)(device_t* this, void* buffer, size_t len, off_t* off);
  /** writes "len" bytes to stream oriented device "this" at position "off" 
   * from buffer "buffer" 
   * and then updates the "off" variable to the next position 
   * returns the number of written bytes or -1 when end of file is reached */
  ssize_t (*write)(device_t* this, const void* buffer, size_t len, off_t* off);

  /** reads len fixed-size blocks from 'start' position (in blocks).
   * It returs 0 on success and -1 on error. All blocks are readed or error is thrown. */
  int (*read_blocks)(device_t* this, block_t* buffer, off_t start, size_t len);
  /** writes len fixed-size blocks from 'start' position (in blocks).
   * It returs 0 on success and -1 on error. All blocks are written or error is thrown. */
  int (*write_blocks)(device_t* this, const block_t* buffer, off_t start, size_t len);

  /** sets a driver specific option */
  int (*set_device_option)(device_t* this, const char* name, va_list args);
  /** gets a driver specific option.
   * return values other than int should be passed by reference */
  int (*get_device_option)(device_t* this, const char* name, va_list args);
};
typedef struct device_ops device_ops_t;

struct device {
  char *name;
  device_ops_t *ops;
  list_head_t children;
  list_head_t siblings;
  device_t *parent;
  driver_t *driver;

  void *private;
};

void device_attach_driver(device_t *device, driver_t *driver);
void device_attach_device(device_t *device, device_t *driver);

/** initializes a device structure to defaults (all null) */
void device_init_device(device_t *device);

int device_get_device_option(device_t *device, const char* name, ...);

function_t device_t_invoke_lookup(device_t *device, invoke_offset_t offset);

// default ops
void device_default_init(device_t* this);

char* device_default_devfs_filename(device_t* this);
void device_default_register_devfs(device_t* this);
int device_default_set_device_option(device_t* this, const char* name, va_list args);
int device_default_get_device_option(device_t* this, const char* name, va_list args);

//int device_set_option(device_t *device, const char* name, va_list args);
//int device_get_option(device_t *device, const char* name, va_list args);

#endif
