#include <application.h>
#include <multiboot.h>
#include <libc.h>
#include <thread.h>
#include <hal.h>

#include <dog.h>
#include <thread_test.h>
#include <pci_test.h>
#include <paging_test.h>
#include <file_test.h>
#include <hd_test.h>
#include <vfs_test.h>
#include <net/pcnet32_test.h>

/** simple app */
#define SAPP(name) { #name, name ## _init, name ## _run, 0, 0 }
/** thread app */
#define TAPP(name) { #name, name ## _init, name ## _run, 1, 0 }
#define STAPP(name,stack) { #name, name ## _init, name ## _run, 1, stack }
/** add applications HERE: */
struct application_t applications[] = {
  TAPP(dog),
  TAPP(thread_test),
  TAPP(pci_test),
  TAPP(paging_test),
  TAPP(file_test),
  STAPP(hd_test, 4096*8),
  STAPP(vfs_test, 4096*20),
// STAPP(pcnet32_test, 4096*20)
};

void applications_init() {
  int i;
  const char *cmdline = multiboot_cmdline();

  for(i=0; i<sizeof(applications)/sizeof(*applications); i++) 
    if(strstr(cmdline, applications[i].name)) 
      applications[i].init();
}

void applications_run() {
  int i;
  struct thread_create_attrs attrs={0,0};

  const char *cmdline = multiboot_cmdline();

  for(i=0; i<sizeof(applications)/sizeof(*applications); i++) 
    
    if(strstr(cmdline, applications[i].name)) {
      attrs.stack_size = applications[i].stack_size;
      if(applications[i].thread)
	thread_create(applications[i].run, 0, &attrs);
      else
	applications[i].run();
    }
}
