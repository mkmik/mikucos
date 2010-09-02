#ifndef _FAT_H_
#define _FAT_H_

#include <filesystem.h>

extern void fat_init();

extern physical_mounted_filesystem_t* fat_physical_mount(filesystem_t* this, 
							 device_t* device, 
							 fnode_t* mountpoint);

extern int fat_read_dir(file_t* this, char* buffer, size_t len, int* flags);
extern int fat_fs_init(mounted_filesystem_t* this);
#endif
