#include <file_test.h>
#include <file.h>
#include <libc.h>
#include <malloc.h>
#include <multiboot.h>
#include <tar.h>

static char* string = "this is my string";
static char buffer[4];

void file_test_init() {
}

void file_test_run() {
  struct file* file;
  size_t res;
  int i;

  struct tar_archive* arch = multiboot_get_module(0);
  arch = tar_find_file(arch, "test.txt");
  if(arch==0) {
    printf("cannot find file 'test.txt'\n");
    return;
  }

  file = open_memfile(tar_file_data(arch), tar_file_size(arch));
  
  printf("Reading file (size %d):\n", strlen(string));
  
  while(1) {
    res = file_read(file, buffer, sizeof(buffer));
    if(res == -1)
      break;
    printf("got %d bytes\n", res);
 
    printf("'");
    for(i=0; i<res; i++)
      putchar(buffer[i]);
    printf("'\n");
    
  }
  printf("got EOF\n");

}
