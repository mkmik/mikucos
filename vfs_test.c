#include <ide.h>
#include <io.h>
#include <libc.h>
#include <hal.h>
#include <timer.h> 
#include <ide.h>

#include <timer.h>
#include <file_api.h>

#include <devfs.h>
#include <tarfs.h>

#include <driver.h>
#include <device.h>
#include <multiboot.h>

#include <shell.h>

void vfs_test_init(){
  
}

void vfs_test_run() {
  ide_init();

  // mounting devfs
  devfs_mount();
  // registering devfs
  driver_register_devfs();

  // mounting tarfs at standard boot location
  tarfs_mount(multiboot_get_module(0), "/System/Startup");

  shell_run();

  xprintf("vfs_test end\n");
}


