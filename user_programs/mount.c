#include "user_programs.h"

#include <stdio.h>
#include <unistd.h>
#include <filesystem.h>

ONLY_INTERNAL;

static void usage() {
  printf("Usage: mount \n"
	 "       mount [OPTION] DEVICE MOUNTPOINT\n"
	 "When used without arguments it prints out the current mount table.\n"
	 "With arguments it mounts a filesystem\n"
	 "\n    -t <type>   filesystem type\n"
	 "\n"
	 "Report bugs to <youknow@noonelistening.net>.\n"
	 );
}

static void print_mount_table() {
  printf("not implemented yet\n");
}

int MAIN(mount, int argc, char** argv) {
  if(argc == 1) {
    print_mount_table();
    return 0;
  } if(argc != 5) {
    usage();
    return 1;
  }

  if(strcmp(argv[1], "-t")) {
    usage();
    return 1;
  }

  if(!filesystem_mount(argv[3], argv[4], argv[2])) {
    printf("cannot mount\n");
    return 1;
  }

  return 0;
}
