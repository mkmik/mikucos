#include <devfs.h>
#include <filesystem.h>
#include <file_api.h>
#include <libc.h>
#include <uart.h>
#include <malloc.h>
#include <process.h>

static struct file_ops devfs_ops = {
  .read = devfs_read,
  .write = devfs_write,
  .read_dir = devfs_read_dir,
  .set_device_option = devfs_set_device_option,
  .get_device_option = devfs_get_device_option
};

static filesystem_t devfs = {
  .name = "devfs",
  .ops = &devfs_ops
};

avl_node_t *id_tree_root;
avl_node_t *device_tree_root;

static devfs_node_t devfs_root = {
  .file_id = 0,
  .name = "",
  .children = LIST_HEAD_INIT(devfs_root.children),
  .siblings = LIST_HEAD_INIT(devfs_root.siblings),
  .id_tree = null_avl_node,
  .device_tree = null_avl_node,
};

static int next_file_id = 1;

void devfs_init() {
  filesystem_register(&devfs);
  avl_add(&devfs_root.id_tree, &id_tree_root, devfs_node_t);
}

void devfs_register_device(char* path, device_t *device) {
  devfs_node_t* parent = &devfs_root;

  /* check. uses vfs, could be done using only devfs api */
  char dirname[256];
  strcpy(dirname, "/Devices/");
  strcat(dirname, path);
  fnode_t *dir = file_resolve_path(dirname);
  
  if(dir) {
    panicf("device already registered!");
  }
  /* end check */

  // look to see if there are parts of the hierarchy that should be built
  char* dirpart = file_path_dirname(path);
  if(dirpart) {
    parent = devfs_ensure_directory_exist(dirpart, parent);
    free(dirpart);
  }

  devfs_node_t *node = malloc(sizeof(devfs_node_t));
  devfs_init_devfs_node(node);
  node->name = file_path_filename(path);
  node->device = device;
  devfs_devfs_node_add(node, parent);

  // free(path) // assume calles assumes we retain it. holy shit c!!!
}

void devfs_init_devfs_node(devfs_node_t* node) {
  node->file_id = next_file_id++;
  node->name = 0;
  node->device = 0;
  INIT_LIST_HEAD(&node->children);
  INIT_LIST_HEAD(&node->siblings);
  node->id_tree = null_avl_node;
  node->device_tree = null_avl_node;
}

void devfs_devfs_node_add(devfs_node_t* node, devfs_node_t* parent) {
  avl_add(&node->id_tree, &id_tree_root, devfs_node_t);
  list_add_tail(&node->siblings, &parent->children);
}

devfs_node_t* devfs_lookup_dir(char* name, devfs_node_t* parent) {
  if(!*name) // root
    return &devfs_root; 

  list_head_t *pos;
  list_for_each(pos, &parent->children) {
    devfs_node_t *it = list_entry(pos, devfs_node_t, siblings);
    assert(it);
    assert(it->name);

    if(strcmp(it->name, name) == 0) {
      return it;
    }
  }
  return 0;
}

devfs_node_t* devfs_create_dir(char* name, devfs_node_t* parent) {
  assert(name);
  assert(strlen(name));

  devfs_node_t *node = malloc(sizeof(devfs_node_t));
  devfs_init_devfs_node(node);
  node->name = name;
  devfs_devfs_node_add(node, parent);  

  assert(devfs_lookup_dir(name, parent));

  return node;
}

devfs_node_t* devfs_ensure_directory_exist(char* path, devfs_node_t* parent) {
  devfs_node_t* newdir;

  char *dirname = file_path_dirname(path);

  // recurse through parents to ensure that they exist. I love recursion :-)
  if(dirname) 
    parent = devfs_ensure_directory_exist(dirname, parent);
  
  char *newname = file_path_filename(path);
  if(!newname)
    newname = "";
  newdir = devfs_lookup_dir(newname, parent);
  if(!newdir)
    newdir = devfs_create_dir(newname, parent);

  assert(newdir);
  return newdir;
}

void devfs_mount() {
  uprintf("mounting devfs\n");
   
  fnode_t *dev = malloc(sizeof(fnode_t));
  debug_memory_lock(dev, "devfs dev");
  dev->mounted = malloc(sizeof(mounted_filesystem_t));
  debug_memory_lock(dev->mounted, "devfs dev->mounted");
  dev->mounted->mountpoint = file_resolve_path("/Devices");
  dev->mounted->root = dev;
  dev->mounted->filesystem = &devfs;
  dev->file_id = 0;
  dev->flags = DIRENT_DIR;

  avl_add(&dev->mounted->mountpoint_tree, &mount_tree, mounted_filesystem_t);
}

int devfs_read(file_t *this, void* buffer, size_t len, off_t *off) {    
  assert(this);
  assert(this->fnode);
  devfs_node_t *node = avl_find(this->fnode->file_id, id_tree_root, devfs_node_t, id_tree);
  assert(node);

  if(node->device) {
    invoked_method_t(device, read) method = invoke(device, node->device, read);
    if(method)
      return method(node->device, buffer, len, off);
  }
  xprintf("device doesn't implement read method\n"); 
  return -1; 
}

int devfs_write(file_t *this, const void* buffer, size_t len, off_t *off) {
  assert(this);
  assert(this->fnode);
  devfs_node_t *node = avl_find(this->fnode->file_id, id_tree_root, devfs_node_t, id_tree);
  assert(node);

  if(node->device) {
    invoked_method_t(device, write) method = invoke(device, node->device, write);
    if(method)
      return method(node->device, buffer, len, off);
  }
  xprintf("device doesn't implement read method\n"); 
  return -1; 
}

int devfs_read_dir(file_t* this, char* buffer, size_t len, int* flags) {
  devfs_node_t *dir = avl_find(this->fnode->file_id, id_tree_root, devfs_node_t, id_tree);

  list_head_t *cur = dir->children.next;
  int i = this->position++;

  *flags = DIRENT_DIR;

  while(i--) {
    cur = cur->next;
    if(cur == &dir->children) { // don't wrap around, it's a circular list!
      *flags = DIRENT_FILE;  // ALL devfs non-leaf files are also directories !
      return -1;
    }
  }
  
  devfs_node_t *node = list_entry(cur, devfs_node_t, siblings); 

  strncpy(buffer, node->name, len);
  
  return node->file_id;
}

int devfs_set_device_option(file_t* this, const char* name, va_list args) {
  assert(this);
  assert(this->fnode);
  devfs_node_t *node = avl_find(this->fnode->file_id, id_tree_root, devfs_node_t, id_tree);
  assert(node);

  if(node->device) {
    invoked_method_t(device, set_device_option) method = invoke(device, node->device, 
								set_device_option);
    if(method)
      return method(node->device, name, args);
  }
  xprintf("device doesn't implement read method\n"); 
  return -1; 
}

int devfs_get_device_option(file_t* this, const char* name, va_list args) {
  assert(this);
  assert(this->fnode);
  devfs_node_t *node = avl_find(this->fnode->file_id, id_tree_root, devfs_node_t, id_tree);
  assert(node);

  if(node->device) {
    invoked_method_t(device, get_device_option) method = invoke(device, node->device, 
								get_device_option);
    if(method)
      return method(node->device, name, args);
  }
  xprintf("device doesn't implement read method\n"); 
  return -1; 
}

device_t* devfs_device_from_file(file_t* file) {
  assert(file);
  assert(file->fnode);
  devfs_node_t *node = avl_find(file->fnode->file_id, id_tree_root, devfs_node_t, id_tree);
  if(!node)
    return 0;

  assert(node->device);

  return node->device;
}
