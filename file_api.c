#include <file_api.h>
#include <file.h>
#include <process.h>
#include <malloc.h>
#include <libc.h>
#include <uart.h>

#include <device.h>
#include <devfs.h>
// hack
#include <ngulfs.h>

ssize_t read(int fd, void* buffer, size_t len) {
  if((fd < 0) || (fd >= MAX_FD))
    return -1;
  file_t* file = process_current()->fd[fd];
  if(!file)
    return -1;

  if(buffer)
    if(!len)
      return -1;
  return file_read(file, buffer, len);
}

ssize_t write(int fd, const void* buffer, size_t len) {
  if((fd < 0) || (fd >= MAX_FD))
    return -1;
  file_t* file = process_current()->fd[fd];
  if(!file)
    return -1;

  return file_write(file, buffer, len);
}

int open(const char *pathname, int flags) {
  fnode_t *file = file_resolve_path(pathname);
  if(!file)
    return -1;
  int fd = file_open_fnode(file, flags);
  return fd;
}

int close(int fd) {
  if((fd < 0) || (fd >= MAX_FD))
    return -1;
  if(!process_current()->fd[fd])
     return -1;
  int r = file_close(process_current()->fd[fd]);
  if(!r)
    process_current()->fd[fd] = 0;

  return 0;
}

int dup2(int oldfd, int newfd) {
 if((oldfd < 0) || (oldfd >= MAX_FD))
   return -1;
 if((newfd < 0) || (newfd >= MAX_FD))
   return -1;

  if(oldfd == newfd)
    return oldfd;

  close(newfd);
  process_current()->fd[newfd] = process_current()->fd[oldfd];
  process_current()->fd[newfd]->reference_count++;

  return newfd; 
}

fnode_t *read_dir(int fd, char* buffer, size_t len) {
  file_t *file = process_current()->fd[fd];
  if(!(file->fnode->flags & DIRENT_DIR))
    return 0;

  fnode_t *dir = malloc(sizeof(fnode_t));
  assert(dir);

  dir->file_id = file->ops->read_dir(file, buffer, len, &dir->flags);
  dir->mounted = file->fnode->mounted;
  
  if(dir->file_id != -1)
    return dir;
  
  free(dir);
  return 0;
}

int set_device_option(int fd, const char* name, ...) {
  va_list args;
  va_start(args, name);

  file_t* file = process_current()->fd[fd];
  assert(file);
  assert(file->ops);
  if(!file->ops->get_device_option)
    return -1;
  return file->ops->set_device_option(file, name, args);
}

int get_device_option(int fd, const char* name, ...) {
  va_list args;
  va_start(args, name);

  file_t* file = process_current()->fd[fd];
  assert(file);
  assert(file->ops);
  if(!file->ops->get_device_option)
    return -1;
  return file->ops->get_device_option(file, name, args);
}
