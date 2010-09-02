#include "user_programs.h"

#include <stdio.h>
#include <unistd.h>

ONLY_INTERNAL;

static void usage() {
  printf("Usage: cat [OPTION] FILE\n"
	 "Prints out the content of the file to stdout.\n"
	 "This is not real cat... it does not conCATenate files...\n"
	 "\n"
	 "Report bugs to <youknow@noonelistening.net>.\n"
	 );
}

int MAIN(cat, int argc, char** argv) {
  const char *path;
  
  if(argc < 2) {
    usage();
    return 1;
  }

  path = argv[1];

  int fd = open(path, 0);
  if(fd < 0) {
    printf("cannot open file or directory\n");
    return 1;
  }
  
  char buffer[256];
  int res;
  while((res = read(fd, buffer, sizeof(buffer))) > 0) {
    write(stdout_fd, buffer, res);
  } 
  
  if(res == -1) {
    // could be EOF but we have no errno!! THIS IS KaOS!! :-)
    //    printf("error reading input\n");
  }
  
  close(fd);

  return 0;
}
