#ifndef _FILE_API_H_
#define _FILE_API_H_

#include <types.h>
#include <file.h>

#define stdin_fd 0
#define stdout_fd 1
#define stderr_fd 3

ssize_t read(int fd, void* buffer, size_t len);
ssize_t write(int fd, const void* buffer, size_t len);

int open(const char *pathname, int flags);
int close(int fd);

/** it makes newfd be the copy of oldfd, closing newfd first if necessary.
 * If oldfd == newfd then do nothing. 
 * It returns the new descriptor, or -1 if an error occurred.
 * Reference count for the file is incremented.
 */
int dup2(int oldfd, int newfd);

fnode_t *read_dir(int fd, char* buffer, size_t len);

int set_device_option(int fd, const char* name, ...);
int get_device_option(int fd, const char* name, ...);

#endif
