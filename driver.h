#ifndef __DRIVER_H_
#define __DRIVER_H_

#include <types.h>
#include <list.h>
#include <avl.h>
#include <libc.h>
#include <invoke.h>

struct driver_ops;
typedef struct driver_ops driver_ops_t;

struct device;
typedef struct device device_t;

/**
 * Each driver in the system has a structure
 * of type driver_t
 */
struct driver {
  char* name;
  driver_ops_t *ops;
  /** null if no parent */
  struct driver *parent;

  /** children sorted by name */
  avl_node_t *children;
  avl_node_t siblings;
  /** linked list of children 
   * for fast enumeration */
  list_head_t children_list;
  list_head_t siblings_list;

  /** devices */
  list_head_t devices;
};
typedef struct driver driver_t;

avl_make_string_compare(driver_t, siblings, name);
avl_make_string_match(driver_t, name);

struct driver_ops {
  void (*init)(driver_t* this);
  char* (*devfs_basedir)(driver_t* this);
  void (*register_devfs)(driver_t* this);
};

void driver_init();

/** registers the driver. 
 * parent slash separated list of driver names
 * which locates a parent appropriate parent. 
 * passing null is the same as "", the root device.
 * returns 0 on success */
int driver_register(driver_t *driver, char *parent);
/** same as above but with a pointer to the parent */
int driver_register_driver(driver_t *driver, driver_t *parent);
/** registers the whole driver tree into devfs.
 * devfs should be mounted */
int driver_register_devfs();
/** returns a driver from a path */
driver_t *driver_find(char* path);

/** initializes a driver structure to defaults (all null) */
void driver_init_driver(driver_t *driver);

function_t driver_t_invoke_lookup(driver_t *driver, invoke_offset_t offset);

extern driver_t root_driver;

#endif
