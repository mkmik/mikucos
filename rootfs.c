#include <rootfs.h>
#include <filesystem.h>
#include <file_api.h>
#include <libc.h>
#include <uart.h>
#include <malloc.h>
#include <process.h>

static struct file_ops rootfs_ops = {
  .read = rootfs_read,
  .write = rootfs_write,
  .read_dir = rootfs_read_dir
};

static filesystem_t rootfs = {
  .name = "rootfs",
  .ops = &rootfs_ops
};

void rootfs_init() {
  filesystem_register(&rootfs);
}

void rootfs_mount() {
  fnode_t *root = malloc(sizeof(fnode_t));
  debug_memory_lock(root, "rootfs root");
  root->mounted = malloc(sizeof(mounted_filesystem_t));
  debug_memory_lock(root->mounted, "rootfs root->mounted");
  root->mounted->mountpoint = root;
  root->mounted->root = root;
  root->mounted->filesystem = &rootfs;
  root->mounted->mountpoint_tree = null_avl_node;
  root->file_id = 0;
  root->flags = DIRENT_DIR;

  avl_add(&root->mounted->mountpoint_tree, &mount_tree, mounted_filesystem_t);

  process_current()->root = root;
}

int rootfs_read(file_t *this, void* buffer, size_t len, off_t *off) {    
  switch(this->fnode->file_id) {
  default:
    return 0;
  }
}

int rootfs_write(file_t *this, const void* buffer, size_t len, off_t *off) {
  switch(this->fnode->file_id) {
  case 10:
  default:
    return -1;
  }
}

int rootfs_read_dir(file_t* this, char* buffer, size_t len, int* flags) {
  *flags = DIRENT_DIR;
  this->position++;

  switch(this->fnode->file_id) {
  case 0:
    switch(this->position) {
    case 1:
      strcpy(buffer, "Applications");
      return 1;
    case 2:
      strcpy(buffer, "Devices");
      return 2;
    case 3:
      strcpy(buffer, "Network");
      return 3;
    case 4:
      strcpy(buffer, "System");
      return 4;
    case 5:
      strcpy(buffer, "Users");
      return 5;
    default:
      return -1;
    }
  case 4:
    switch(this->position) {
    case 1:
      strcpy(buffer, "Startup");
      return 101;
    case 2:
      strcpy(buffer, "tmp");
      return 102;
    case 3:
      strcpy(buffer, "mnt");
      return 103;
    default:
      return -1;
    }
  default:
    return -1;
  }
}
