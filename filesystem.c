#include <filesystem.h>
#include <file.h>
#include <devfs.h>

#include <string.h>
#include <malloc.h>

LIST_HEAD(filesystems_head);
avl_node_t *mount_tree = 0;

void filesystem_register(filesystem_t* fs) {
  list_add(&fs->fs_list, &filesystems_head);
}

filesystem_t* filesystem_find(const char* name) {
  list_head_t *pos;
  list_for_each(pos, &filesystems_head) {
    filesystem_t *fs = list_entry(pos, filesystem_t, fs_list);
    if(strcmp(fs->name, name) == 0)
      return fs;
  }
  return 0;
}

extern int filesystem_mount(const char* devicename, 
			    const char* mountpoint,
			    const char* type) {

  filesystem_t *fs = filesystem_find(type);
  if(!fs) {
    xprintf("cannot find specified filesystem '%s'", type);
    return 0;
  }
    
  fnode_t *mnode = file_resolve_path(mountpoint);
  if(!mnode) {
    xprintf("mountpoint %s doesn't exist\n", mountpoint);
    return 0;
  }
  if(!(mnode->flags & DIRENT_DIR)) {
    xprintf("mountpoint %s is not a directory\n");
    free(mnode);
    return 0;
  }

  fnode_t *dnode = file_resolve_path(devicename);
  if(!dnode) {
    xprintf("device %s doesn't exist\n", devicename);
    free(mnode);
    return 0;
  }
  
  file_t *dfile = file_open_fnode_file(dnode, 0);
  if(!dfile) {
    xprintf("cannot open device %s\n", devicename);
    free(dnode);
    free(mnode);
    return 0;
  }
  
  device_t *device = devfs_device_from_file(dfile);
  if(!device) {
    xprintf("%s is not a device!\n", devicename);
    free(dnode);
    free(mnode);
    file_close(dfile);
    return 0;
  }

  physical_mounted_filesystem_t *mfs;
  int has_mount = 0;
  if(fs->fops)
    if(fs->fops->physical_mount)
      has_mount = 1;
  if(has_mount)
    mfs = fs->fops->physical_mount(fs, device, mnode);
  else
    mfs = calloc(1, sizeof(physical_mounted_filesystem_t));
    
  if(!mfs) {
    xprintf("cannot mount filesystem\n");
    free(dnode);
    free(mnode);
    file_close(dfile);
    return 0;
  }
  if(!mfs->mountpoint)
    mfs->mountpoint = mnode;
  if(!mfs->filesystem)
    mfs->filesystem = fs;
  if(!mfs->device)
    mfs->device = device;

  if(!mfs->root)
    mfs->root = filesystem_create_default_root_node((mounted_filesystem_t*)mfs);  

  int has_init = 0;
  if(fs->fops)
    if(fs->fops->init)
      has_init = 1;
  if(has_init)
    if(!fs->fops->init((mounted_filesystem_t*)mfs)) {
      xprintf("cannot mount filesystem, bad information on disk\n");
      free(dnode);
      free(mnode);
      file_close(dfile);
      free(mfs);
      return 0;
    }

  // physical_mounted_filesystem inherits mounted_filesystem_t and all
  avl_add(&mfs->mountpoint_tree, &mount_tree, mounted_filesystem_t);

  return 1;
}

fnode_t* filesystem_create_default_root_node(mounted_filesystem_t* mfs) {
  fnode_t *root = calloc(1, sizeof(fnode_t));
  root->mounted = mfs;
  root->file_id = 0;
  root->flags = DIRENT_DIR;
  return root;
}

int filesystem_read_clusters(physical_mounted_filesystem_t* this, fcluster_t* buffer, 
			     off_t start, size_t len) {  
  return invoke(device, this->device, read_blocks)(this->device, buffer, 
						   start * this->cluster_size + 
						   this->clustered_data_start,
						   len * this->cluster_size);
}

extern int filesystem_read_clusters_unaligned(physical_mounted_filesystem_t* this, 
					      fcluster_t* buffer, 
					      off_t start, size_t len) {
  return invoke(device, this->device, read_blocks)(this->device, buffer, 
						   start + this->clustered_data_start,
						   len * this->cluster_size);
}

extern int filesystem_read_blocks(physical_mounted_filesystem_t* this, 
				  block_t* buffer, 
				  off_t start, size_t len) {
  return invoke(device, this->device, read_blocks)(this->device, buffer, 
						   start,
						   len);
}
