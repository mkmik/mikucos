#ifndef _PCNET32_H_
#define _PCNET32_H_

#include <driver.h>
#include <device.h>

void pcnet32_init();

int pcnet32_get_device_option(device_t* this, const char* name, va_list args);
int pcnet32_set_device_option(device_t* this, const char* name, va_list args);
ssize_t pcnet32_read(device_t* this, void* buffer, size_t len, off_t* off );
ssize_t pcnet32_write(device_t* this, const void* buffer, size_t len, off_t* off );
#endif

