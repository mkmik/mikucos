#ifndef __TARFS_H_
#define __TARFS_H_

#include <types.h>
#include <filesystem.h>
#include <tar.h>

void tarfs_init();
void tarfs_mount(tar_archive_t *tar, const char* mountpoint);
int tarfs_read(file_t *this, void* buffer, size_t len, off_t *off);
int tarfs_read_dir(file_t* this, char* buffer, size_t len, int* flags);

#endif
