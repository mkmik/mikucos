#ifndef __ROOTFS_H_
#define __ROOTFS_H_

#include <types.h>
#include <file.h>

void rootfs_init();
void rootfs_mount();
int rootfs_read(file_t *this, void* buffer, size_t len, off_t *off);
int rootfs_write(file_t *this, const void* buffer, size_t len, off_t *off);
int rootfs_read_dir(file_t* this, char* buffer, size_t len, int* flags);

#endif
