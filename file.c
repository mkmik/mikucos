#include <file.h>
#include <filesystem.h>
#include <malloc.h>
#include <libc.h>
#include <uart.h>
#include <process.h>
#include <process_api.h>
#include <file_api.h>

ssize_t file_read(file_t *this, void* buffer, size_t len) {
  assert(this);
  assert(this->ops);
  assert(this->ops->read);
  return this->ops->read(this, buffer, len, &this->position);
}

ssize_t file_write(file_t *this, const void* buffer, size_t len) {
  assert(this);
  assert(this->ops);
  assert(this->ops->read);
  return this->ops->write(this, buffer, len, &this->position);
}

file_t *file_open_fnode_file(fnode_t* fnode, int flags) {
  file_t *file = malloc(sizeof(file_t));
  file->ops = fnode->mounted->filesystem->ops;
  file->position = 0;
  file->flags = flags;
  file->fnode = fnode;
  file->reference_count = 1;

  return file;
}

int file_open_fnode(fnode_t* fnode, int flags) {
  file_t *file = file_open_fnode_file(fnode, flags);
  int fd = file_find_next_fd();
  if(fd == -1)
    panicf("no free file descriptors");

  process_current()->fd[fd] = file;
  return fd;
}

int file_close(file_t* file) {
  file->reference_count--;
  int r = file->reference_count;
  if(!r)
    free(file);
  return r;
}

int file_find_next_fd() {
  file_t** fd = process_current()->fd;
  int i;
  for(i=0; i<MAX_FD; i++)
    if(fd[i] == 0)
      return i;
  return -1;
}

fnode_t *file_resolve_path(const char* path) {
  if(strcmp(path, "/") == 0)
    return process_current()->root;
  if(path[0] == '/')
    return file_resolve_relative_path(path+1, process_current()->root);
  else {
    char buffer[512];
    getcwd(buffer, sizeof(buffer));
    if(buffer[strlen(buffer) - 1] != '/')
      strcat(buffer, "/");
    strcat(buffer, path);
    return file_resolve_path(buffer);
  }
}

static char pname[256];
static char ename[256];

fnode_t *file_resolve_relative_path(const char* path, fnode_t* parent) {
  const char* pend = path;

  while(*pend != '/' && *pend != 0)
    pend++;

  memcpy(pname, path, pend-path);
  pname[pend-path] = 0;

  int fd = file_open_fnode(parent, 0);
  fnode_t *file;
  while((file = read_dir(fd, ename, sizeof(ename)))) {
    if(strcmp(pname, ename) == 0) 
      break;
    free(file);
  }
  close(fd);

  // if exists
  if(file) {
    // file
    if(file->flags & DIRENT_FILE) {
      if(*pend == '/') {
	xprintf("%s: Not a directory", pname);
	return 0;
      }
      return file;
    } else {
      // dir
      mounted_filesystem_t *mount = avl_find(file, mount_tree, 
					     mounted_filesystem_t, mountpoint_tree);

      if(mount) {
	free(file);
	file = mount->root;
      } 
      
      
      if(*pend != '/') 
	return file;
      
      pend++;
      fnode_t* res = file_resolve_relative_path(pend, file);
      
      // free(file); // THIS FREE HAS PROBLEMS
      return res;
    }
  }
  // error
  //  uprintf("%s: No such file or directory\n", pname);
  return 0;
}

char* file_path_dirname(const char* path) {
  const char *p = path;
  const char *last = 0;
  while(*p)
    if(*p == '/')
      last = p++;
    else
      p++;

  if(!last)
    return 0;
  
  char *res = malloc(last - path + 1);
  strncpy(res, path, last - path);
  res[last - path] = 0;
  return res;
}

char* file_path_filename(const char* path) {
 const char *p = path;
 const char *last = p;
 const char *end = path + strlen(path);
 while(*p)
   if(*p == '/')
     last = p++;
   else
     p++;

 if(*last == '/')
   last++;

 if(last == end)
   return 0;

 char *res = malloc(end - last + 1);
 strncpy(res, last, end - last);
 res[end - last] = 0;
 return res;
}

char* file_path_firstitem(const char* path) {
  const char *p = path;
  while(*p && (*p != '/'))
    p++;
    
  char *res = malloc(p - path + 1);
  strncpy(res, path, p - path);
  res[p - path] = 0;
  return res;
}

char* file_path_dropfirst(const char* path) {
  const char *p = path;
  const char *end = path + strlen(path);
  while(*p && (*p != '/'))
    p++;

  if(*p == '/')
    p++;

  char *res = malloc(end - p + 1);
  strncpy(res, p, end - p);
  res[end - p] = 0;
  return res;
}

int file_set_device_option(file_t* file, const char* name, ...) {
  va_list args;
  va_start(args, name);

  assert(file);
  assert(file->ops);
  if(!file->ops->get_device_option)
    return -1;
  return file->ops->set_device_option(file, name, args);
}

int file_get_device_option(file_t* file, const char* name, ...) {
  va_list args;
  va_start(args, name);

  assert(file);
  assert(file->ops);
  if(!file->ops->get_device_option)
    return -1;
  return file->ops->get_device_option(file, name, args);
}

//////////

ssize_t memfile_read(file_t *this, void* buffer, size_t len, off_t *off) {
  struct memfile_private * private= (struct memfile_private *)this->private;

  if(private->size - *off <= 0)
    return -1;

  if(private->size-*off < len) 
    len = private->size-*off;
  
  memcpy(buffer, private->data + *off, len);
  *off += len;
  return len;
}

ssize_t memfile_write(file_t *this, const void* buffer, size_t len, off_t *off) {
  //  struct memfile_private * private= (struct memfile_private *)this->private;
  
  return 0;
}

struct file_ops memfile_ops = {
  .read = memfile_read,
  .write = memfile_write
};

file_t *open_memfile(void *data, size_t len) {
  file_t* file = hmalloc(sizeof(file_t));
  struct memfile_private* private = hmalloc(sizeof(struct memfile_private));

  file->ops = &memfile_ops;
  file->position = 0;
  file->private = private;
  private->data = data;
  private->size = len;

  return file;
}
