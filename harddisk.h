#ifndef _HARDDISK_H_
#define _HARDDISK_H_

#include <driver.h>

void harddisk_init();

// ops
void harddisk_register_devfs(driver_t* this);

#endif
