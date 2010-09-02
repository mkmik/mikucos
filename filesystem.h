#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include <avl.h>
#include <device.h>
#include <block.h>
#include <file.h>

struct filesystem_ops;

/**
 * Every registered filesystem has a filesystem structure 
 */

struct filesystem {
  const char* name;
  file_ops_t *ops;
  struct filesystem_ops *fops;
  list_head_t fs_list;
};

typedef struct filesystem filesystem_t;

void filesystem_register(filesystem_t* fs);

/**
 * Every mounted filesystem has a mount structure
 */
struct mounted_filesystem {
  filesystem_t *filesystem;
  fnode_t *mountpoint;
  fnode_t *root;
  void *private;
  avl_node_t mountpoint_tree;
};

typedef struct mounted_filesystem mounted_filesystem_t;

static inline 
int mounted_filesystem_t_avlcompare(avl_node_t *_a, avl_node_t *_b) {   
  mounted_filesystem_t *a = avl_entry(_a, mounted_filesystem_t, mountpoint_tree); 
  mounted_filesystem_t *b = avl_entry(_b, mounted_filesystem_t, mountpoint_tree); 
  
  int fs = a->mountpoint->mounted < b->mountpoint->mounted 
    ? -1 : (a->mountpoint->mounted < b->mountpoint->mounted ? 0 : 1);
  if(fs == 0)
    return a->mountpoint->file_id < b->mountpoint->file_id 
      ? -1 : (a->mountpoint->file_id < b->mountpoint->file_id ? 0 : 1);
  return fs;
}

static inline 
int mounted_filesystem_t_avlmatch(fnode_t* k,
				  mounted_filesystem_t *b) {

  int fs = k->mounted < b->mountpoint->mounted 
    ? -1 : (k->mounted == b->mountpoint->mounted ? 0 : 1);
  if(fs == 0)
    return k->file_id < b->mountpoint->file_id 
      ? -1 : (k->file_id == b->mountpoint->file_id ? 0 : 1);
  return fs;
}

/*
 * the mounted_filesystem is too generic.
 * This one is an extension of it for use with normal
 * device-->filesystem  one to one mappings common
 * in normal filesystems. (fat, ngulfs, ext2, iso9660, ...)
 */
struct physical_mounted_filesystem {
  //  mounted_filesystem_t; // inherit
  filesystem_t *filesystem;
  fnode_t *mountpoint;
  fnode_t *root;
  void *private;
  avl_node_t mountpoint_tree;
  // end mounted_filesystem
  device_t* device;
  /** number of sectors in cluster */
  size_t cluster_size;
  /** sector where data organized in clusters begin.
   * can be also negative is not important. No check is performed */
  int32_t clustered_data_start;
};

typedef struct physical_mounted_filesystem physical_mounted_filesystem_t;

/** filesystem operations.
 * those are used on a mounted filesystem object or filesystem object.
 * Implement those if you want the filesystem to be mountable using standard functions.
 * A special filesystem can still work without some or all there methods (tarfs, for ex).
 */
struct filesystem_ops {
  /**
   * this method is called when the mounted filesystem is allocated
   * but not yet inserted in the mount tree.
   *  place here low level mount functions like reading super block, mft, fat, etc
   * return non zero if succesful or 0 if some error occours
   */
  int (*init)(mounted_filesystem_t* this);
  /** 
   * device and mountpoint are already checked for existence.
   * return the newly allocated mounted_filesystem_t structure
   * which will be automatically added to the tree of mounted filesystems. 
   *  You don't need to setup some obvious fields (mountpoint and filesystem)
   * of this structure since 
   * this will be done for you (if you leave it 0) by filesystem_mount.
   *  Remember to zero fill the structure as you allocate it! (with calloc for ex) 
   */
  physical_mounted_filesystem_t* (*physical_mount)(filesystem_t* this, 
						   device_t* device, 
						   fnode_t* mountpoint);
  
};
typedef struct filesystem_ops filesystem_ops_t;


// global variables 

extern avl_node_t *mount_tree; // TODO: make it private

// API

/** if type is 0 the try to find it out automatically.
 * return 1 if successful */
extern int filesystem_mount(const char* devicename, 
			    const char* mountpoint,
			    const char* type);

extern filesystem_t* filesystem_find(const char* name);

extern fnode_t* filesystem_create_default_root_node(mounted_filesystem_t* mfs);

typedef block_t fcluster_t;

/** reads clusters from a cluster aligned sector address.
 * 'start' is cluster number */
extern int filesystem_read_clusters(physical_mounted_filesystem_t* this, 
				    fcluster_t* buffer, 
				    off_t start, size_t len);

/** reads clusters from a non-cluster aligned sector address.
 * 'start' is sector number */
extern int filesystem_read_clusters_unaligned(physical_mounted_filesystem_t* this, 
					      fcluster_t* buffer, 
					      off_t start, size_t len);

/** reads sectors.
 * 'start' is sector number 
 * len in blocks */
extern int filesystem_read_blocks(physical_mounted_filesystem_t* this, 
				  block_t* buffer, 
				  off_t start, size_t len);
#endif
