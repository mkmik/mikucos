#include <dog.h>
#include <tar.h>
#include <multiboot.h>
#include <hal.h>
#include <video.h>
#include <libc.h>
#include <uart.h>
#include <timer.h>

static char *dog[20]; // hack
static int dog_len[20];
static int dogs;

void dog_init() {
  struct tar_archive* arch = multiboot_get_module(0);
  if(!arch) {
    uprintf("dog: no tar file\n");
    return;
  }
  
  uprintf("reading tar from %p...\n", arch);
  while(tar_is_valid_file(arch)) {
    if(arch->block[0].header.typeflag == REGTYPE) {
      uprintf(" %s\n", arch->block[0].header.name);
      dog[dogs] = tar_file_data(arch);
      dog_len[dogs++] = tar_file_size(arch);
    } 
    arch = tar_next_file(arch);
  }
  uprintf("done...\n\n");
}

void dog_run() {
  while(1) {      
    dog_draw();
    delay(1000);
  }
}

void dog_draw() {
  static unsigned int i=0;
  int this_dog;

  irql_t saved_irql = hal_irql_set(IRQL_DISPATCH);  
  if(dogs) {
    this_dog = (i++)%dogs;
    video_drawAA(dog[this_dog], getRow()+1, 0, dog_len[this_dog]);
  }
  hal_irql_restore(saved_irql);
}
