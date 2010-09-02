#include "user_programs.h"

#include <stdio.h>

ONLY_INTERNAL;

void MAIN(vesa, int argc, char** argv) {
  printf("VESA test\n");
  printf("Scanning BIOS\n");

  char* bios= (char*)0xc0000;
  int found=0;
  do {
    if(strncmp(bios, "PMID", 4) == 0) {
      printf("Found at %p\n", bios);
      found = 1;
    }
  } while(bios++ < ((char*)0xc0000+128*1024));

}
