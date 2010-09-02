#include <pci.h>
#include <net/pcnet32.h>
#include <net/ip.h>
#include <net/ether.h>
#include <ide.h>
#include <libc.h>
#include <hal.h>
#include <ide.h>

#include <malloc.h>
#include <thread.h>
#include <process.h>
#include <mutex.h>
#include <timer.h>
#include <filesystem.h>
#include <file_api.h>

#include <devfs.h>
#include <tarfs.h>

#include <driver.h>
#include <device.h>
#include <shell.h>
#include <multiboot.h>

void pcnet32_test_init(){
  
}

void pcnet32_test_run() {
  xprintf("PCNET32 TEST\n");
  xprintf("root fnode %p\n", process_current()->root);
  xprintf("root filesystem name %s\n", process_current()->root->mounted->filesystem->name);
  xprintf("file id %d\n", process_current()->root->file_id);

  pci_init();
  //  ide_init();
  ether_init();
  ip_init();
  pcnet32_init();   //pcnet32 
  // mounting devfs
  devfs_mount();
  // registering devfs
  driver_register_devfs();

  // mounting tarfs at standard boot location
  tarfs_mount(multiboot_get_module(0), "/System/Startup");
  
  shell_run();

  xprintf("pcnet32_test end\n");
}


