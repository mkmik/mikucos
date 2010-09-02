#include <tarfs.h>
#include <file.h>
#include <file_api.h>
#include <libc.h>
#include <uart.h>
#include <malloc.h>
#include <process.h>

// inherit code from devfs!!!!!
// TODO rename logic behind devfs_nodes in a separate common module
#include <devfs.h>

struct tarfs_node {
  //  devfs_node_t;
  int file_id;
  device_t *device;
  char *name;

  list_head_t children;
  list_head_t siblings;

  /** all nodes sorted per file_id */
  avl_node_t id_tree;
  /** all nodes sorted per device */
  avl_node_t device_tree;

  // end devfs_node
  char* data;
  size_t size;
};
typedef struct tarfs_node tarfs_node_t;

struct tarfs_private {
  list_head_t files;
};
typedef struct tarfs_private tarfs_private_t;
#define private(arg) ((tarfs_private_t*)arg->private)

static struct file_ops tarfs_ops = {
  .read = tarfs_read,
  .read_dir = tarfs_read_dir
};

static filesystem_t tarfs = {
  .name = "tarfs",
  .ops = &tarfs_ops
};

static void tarfs_populate(mounted_filesystem_t* this, tar_archive_t *tar);

void tarfs_init() {
  filesystem_register(&tarfs);
}

void tarfs_mount(tar_archive_t *tar, const char* mountpoint) {
  if(!tar_is_valid_file(tar)) {
    xprintf("You attempted to mount an invalid tar file to %s\n", mountpoint);
    return;
  }

  fnode_t *root = malloc(sizeof(fnode_t));
  mounted_filesystem_t *this = malloc(sizeof(mounted_filesystem_t));
  this->mountpoint = file_resolve_path(mountpoint);
  assert(this->mountpoint);
  this->root = root;
  this->filesystem = &tarfs;
  this->mountpoint_tree = null_avl_node;
  root->mounted = this;
  root->file_id = 0;
  root->flags = DIRENT_DIR;
  this->private = malloc(sizeof(tarfs_private_t));
  INIT_LIST_HEAD(&private(this)->files);

  avl_add(&this->mountpoint_tree, &mount_tree, mounted_filesystem_t);

  tarfs_populate(this, tar);
}

int tarfs_read(file_t *this, void* buffer, size_t len, off_t *off) {  
  list_head_t *pos;
  
  list_for_each(pos, &private(this->fnode->mounted)->files) {
    tarfs_node_t *node = list_entry(pos, tarfs_node_t, siblings);
    if(node->file_id == this->fnode->file_id) {
      
      if(node->size - *off <= 0)
	return -1;
      
      if(node->size-*off < len) 
	len = node->size-*off;
      
      memcpy(buffer, node->data + *off, len);
      *off += len;
      return len;
    }
  }
  return -1;
}

int tarfs_read_dir(file_t* this, char* buffer, size_t len, int* flags) {
  list_head_t *pos;
  *flags = DIRENT_FILE;

  switch(this->fnode->file_id) {
  case 0:
    list_for_each(pos, &private(this->fnode->mounted)->files) {
      tarfs_node_t *node = list_entry(pos, tarfs_node_t, siblings);
      if(node->file_id == (this->position + 1)) {
	strncpy(buffer, node->name, len);
	return (this->position++) + 1; // some gcc -O cannot do preincrement !
      }
    }
  default:
    return -1;
  }
}

static void tarfs_populate(mounted_filesystem_t* this, tar_archive_t *tar) {
  int id = 1;
  do {
    uprintf("file name '%s', size %d bytes\n", tar_file_name(tar), tar_file_size(tar));
    if(strstr(tar_file_name(tar), "/")) {
      uprintf("tarfs subdirectories not yet implemented: skipping\n");
      continue;
    } 
    
    tarfs_node_t *node = malloc(sizeof(tarfs_node_t));
    node->name = tar_file_name(tar);
    node->data = tar_file_data(tar);
    node->size = tar_file_size(tar);
    node->file_id = id++;
    list_add(&node->siblings, &private(this)->files);
    
  } while(tar_is_valid_file(tar = tar_next_file(tar)));
}
