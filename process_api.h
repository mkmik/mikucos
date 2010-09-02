#ifndef _PROCESS_API_H_
#define _PROCESS_API_H_

#include <types.h>

char *getcwd(char *buf, size_t size);

// syscalls

int __syscall_getcwd(char *buf, size_t size);

#endif
