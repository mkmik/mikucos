#include "user_programs.h"

#include <stdio.h>
#include <unistd.h>
#include <malloc.h>

static char path[512];

ONLY_INTERNAL;

int MAIN(ls, int argc, char** argv) {

  if(getcwd(path, sizeof(path)) == 0) {
    printf("errors in getcwd\n");
    return 1;
  }

  int fd = open(path, 0);
  if(fd < 0) {
    printf("cannot open file or directory\n");
    return 1;
  }
  
  char buffer[256];
  fnode_t *dir;
  while((dir = read_dir(fd, buffer, sizeof(buffer)))) {
    if(dir->flags & DIRENT_DIR)
      printf("\e[01;34m");
    printf("%s", buffer);
    printf("\e[00m\n");
    free(dir);
  } 
  
  close(fd);

  return 0;
}
