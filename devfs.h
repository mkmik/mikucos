#ifndef __DEVFS_H_
#define __DEVFS_H_

#include <types.h>
#include <file.h>
#include <device.h>

struct devfs_node {
  int file_id;
  device_t *device;
  char *name;

  list_head_t children;
  list_head_t siblings;

  /** all nodes sorted per file_id */
  avl_node_t id_tree;
  /** all nodes sorted per device */
  avl_node_t device_tree;
};

typedef struct devfs_node devfs_node_t;
/** used to search the AVL tree with the 'device' key */
typedef devfs_node_t devfs_node_device_t;

avl_make_compare(devfs_node_t, id_tree, file_id);
avl_make_match(devfs_node_t, file_id);

/** used to search the AVL tree with the 'device' key */
avl_make_compare(devfs_node_device_t, device_tree, device);
avl_make_match(devfs_node_device_t, device);

void devfs_init();
void devfs_mount();

void devfs_register_device(char* path, device_t *device);

int devfs_read(file_t *this, void* buffer, size_t len, off_t *off);
int devfs_write(file_t *this, const void* buffer, size_t len, off_t *off);
int devfs_read_dir(file_t* this, char* buffer, size_t len, int* flags);
int devfs_set_device_option(file_t* this, const char* name, va_list args);
int devfs_get_device_option(file_t* this, const char* name, va_list args);

/** returns a device from a file 
 * kernel process can then directly work with the device 
 * thus accessing more functionality and performance but 
 * less flexibility and transparency. */
device_t* devfs_device_from_file(file_t* file);

// private

/** initializes an freshly allocated devfs_node_t structure.
 * also allocates an unique id. */
void devfs_init_devfs_node(devfs_node_t* node);

/** add a node to another node (parent).
 * also adds it to the global id search tree. */
void devfs_devfs_node_add(devfs_node_t* node, devfs_node_t* parent);

/** makes sure that the directory with given path starting from 'parent'
 * exists (if not then creates every single directory in the path).
 * it returns the node of the directory pointed with 'path' (the righest).*/
devfs_node_t *devfs_ensure_directory_exist(char* path, devfs_node_t* parent);

/** create a directory */
devfs_node_t* devfs_create_dir(char* name, devfs_node_t* parent);

/** searches for a named children in a node ('parent').
 * returns 0 if it cannot be found */
devfs_node_t* devfs_lookup_dir(char* name, devfs_node_t* parent);

#endif
