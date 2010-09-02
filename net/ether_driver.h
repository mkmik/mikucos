#ifndef _ETHER_H_
#define _ETHER_H_

#include <driver.h>
#include <device.h>

void ether_init();
void ether_device_init();

char* ether_devfs_basedir(driver_t* this);

#endif

