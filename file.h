#ifndef _FILE_H_
#define _FILE_H_

#include <types.h>
#include <list.h>
#include <avl.h>
// hack
#include <libc.h>
#include <stdarg.h>

struct file;

/** file operations.
 * an object-oriented way to make abstract types. 
 * every stuct file points to a file operation object 
 * which provides specific handlers for that type of file. */
struct file_ops {
  /** reads "len" bytes from file "this" at position "off" to buffer "buffer" 
   * and then updates the "off" variable to the next position.
   * returns the number of readed bytes or -1 when end of file is reached */
  ssize_t (*read)(struct file *this, void* buffer, size_t len, off_t *off);
  /** writes "len" bytes to file "this" at position "off" from buffer "buffer" 
   * and then updates the "off" variable to the next position 
   * returns the number of written bytes or -1 when end of file is reached */
  ssize_t (*write)(struct file *this, const void* buffer, size_t len, off_t *off);
  /** returns in buffer the next file name in the directory (max len chars)
   * and the file number. If no more files then return -1 
   */
  int (*read_dir)(struct file* this, char* buffer, size_t len, int* flags);
  /** sets a driver specific option */
  int (*set_device_option)(struct file* this, const char* name, va_list args);
  /** gets a driver specific option.
   * return values other than int should be passed by reference */
  int (*get_device_option)(struct file* this, const char* name, va_list args);
};
typedef struct file_ops file_ops_t;

/** **/
#define DIRENT_FILE  (1<<0)
#define DIRENT_DIR   (1<<1)

struct mounted_filesystem;

/** 
 * System caches path lookup information in fnode structures
 */

struct fnode {
  struct mounted_filesystem *mounted;
  int file_id;
  int flags;
};

typedef struct fnode fnode_t;

/** 
 * Every open file in the system is described with a structure of type
 * "struct file". This structure identifies 
 */
struct file {
  /** file operations */
  struct file_ops *ops;
  /** current file position */
  off_t position;
  /** standard flags (read/write etc) */
  int flags;
  /** fnode */
  fnode_t *fnode;
  /** reference count.
   * open sets it to 1. 
   * dup (not implemented) increments it */
  int reference_count;
  /** private data */
  void *private;
};

typedef struct file file_t;

ssize_t file_read(file_t *this, void* buffer, size_t len);
ssize_t file_write(file_t *this, const void* buffer, size_t len);

/** returns a file pointer 
 * should be used in kernel parts that are independent of the 
 * calling process, because the file descriptor table is 
 **/
file_t *file_open_fnode_file(fnode_t* fnode, int flags);
/** returns a file descriptor */
int file_open_fnode(fnode_t* file, int flags);
int file_find_next_fd();

/** decrements the open counter, and if it drops to 0
 * then deallocates the file.
 * It returns the remaining reference count (0 if deallocated) */
int file_close(file_t* file);

fnode_t *file_resolve_path(const char* path);
fnode_t *file_resolve_relative_path(const char* path, fnode_t* parent);

/** return a newly allocated string containing the dir part of the path.
 * or NULL if it doesn't exist */
char* file_path_dirname(const char* path);
/** return a newly allocated string containing the file part of the path.
 * or NULL if it doesn't exist */
char* file_path_filename(const char* path);

/** return a newly allocated string containing the fist string up to / */
char* file_path_firstitem(const char* path);
/** return a newly allocated string without the fist string up to / */
char* file_path_dropfirst(const char* path);

int file_set_device_option(file_t* file, const char* name, ...);
int file_get_device_option(file_t* file, const char* name, ...);

/// memory file
// this is a simple file driver which emulates a file 
// using a ram buffer. It can be used with little effort to simulate
// disk files using tar archives loaded as multiboot modules (see file_test.c)

/** it contains the pointer to the resident memory buffer 
 * and the size of this buffer */
struct memfile_private {
  u_int8_t *data;
  size_t size;
};

/** implements the file_operations "read" method */ 
ssize_t memfile_read(file_t *this, void* buffer, size_t len, off_t *off);
/** implements the file_operations "write" method */ 
ssize_t memfile_write(file_t *this, const void* buffer, size_t len, off_t *off);

/** creates a new file object backed by the memfile driver.
 * TODO: more abstract "open" technique using "filesystem" drivers */
file_t *open_memfile(void *data, size_t len);

#endif
