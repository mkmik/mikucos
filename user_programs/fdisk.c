#include "user_programs.h"

#include <stdio.h>

#include <device.h>
#include <devfs.h>
#include <block.h>
#include <mbr.h>
#include <process.h>

ONLY_INTERNAL;

static block_space_t block;

static struct {
  int type;
  char* name;
} part_types[] = {
  {DOS_PART_FAT16, "FAT16"},
  {DOS_PART_EXT, "Extended"},
  {DOS_PART_KAOS, "KAOS"},
  {DOS_PART_LINUX, "Linux"},
  {DOS_PART_LINUX_SWAP, "Linux swap"}
};

static char* part_type_name(int id) {
  int i;
  for(i=0; i<sizeof(part_types)/sizeof(part_types[0]); i++)
    if(id == part_types[i].type)
      return part_types[i].name;
  return "unknown";
}

static int read_partition_table(const char* path) {
  mbr_t *mbr = (mbr_t*)&block;

  int fd = open(path, 0);
  if(fd == -1) {
    printf("error %d\n", fd);
    return 1;
  }
  
  device_t* device = devfs_device_from_file(process_current()->fd[fd]);
  int size = get_device_option(fd, "device_size");
  char model[100];
  get_device_option(fd, "model", model, sizeof(model));

  int human = size;
  char *suffix[] = {"", "k", "M", "G", "T", 0};
  int suffix_idx = 0;
  while(human >= 1024) {
    human /= 1024;
    suffix_idx++;
    if(suffix[suffix_idx] == 0)
      break;
  }
  printf("\nDisk %s: %d %sB, %d bytes\n", path, human, suffix[suffix_idx], size);
  printf("Model: '%s'\n\n", model);
  printf("Partition Boot    Start       End    Blocks   Id  System\n");
  
  invoke(device, device, read_blocks)(device, block, 0, 1);
  
  if(mbr->signature != 0xaa55)
    printf("invalid mbr signature\n");

  int i;
  for(i=0; i<4; i++) {
    if((mbr->pe[i].type == 0 ) && (mbr->pe[i].lba_sector == 0))
      continue;
    printf("%d        %c   %10d%10d%10d   %02x  %s\n",
	   i, 
	   mbr->pe[i].active ? '*' : ' ',
	   mbr->pe[i].lba_sector,
	   mbr->pe[i].lba_sector + mbr->pe[i].lba_len,
	   mbr->pe[i].lba_len,
	   mbr->pe[i].type,
	   part_type_name(mbr->pe[i].type));
  }
  int extid = 5;
  int j;
  for(j=0; j<4; j++) {
    if((mbr->pe[j].type == 0 ) && (mbr->pe[j].lba_sector == 0))
      continue;
    if(mbr->pe[j].type == DOS_PART_EXT) {
      printf("found an extended part table (starting at %d)\n", extid);
    }
  }
  
  printf("\n");
  
  return 0;
}

static void print_help() {
  printf("Command action\n"
	 "   m   print this menu\n"
	 "   p   print the partition table\n"
	 "   q   quit without saving changes\n");
}

static int menu(const char* path) {
  static const void *prompt = "\nCommand (m for help): ";
  char cmd[256];
  while(1) {
    printf(prompt);
    
    int res = read(stdin_fd, cmd, sizeof(cmd));
    if(res == -1) {
      printf("error reading command\n");
      return 1;
    }

    printf("\n");
    
    if(cmd[res-1] == '\n')
      cmd[res-1] = 0;
    else // strange, non canonic input ...
      cmd[res] = 0;

    if(strcmp(cmd, "p") == 0)
      read_partition_table(path);
    else if(strcmp(cmd, "m") == 0)
      print_help();
    else if(strcmp(cmd, "q") == 0)
      return 0;
    else 
      printf("%s: unknown command\n", cmd);

  }
}

int MAIN(fdisk, int argc, char** argv) {
  const char *path = "/Devices/Harddisk/IDE/0";  

  if(argc > 1) {
    path = argv[1];
  }

  // test existence
  int fd = open(path, 0);
  if(fd == -1) {
    printf("cannot open %s\n", path);
    return 1;
  }
  close(fd);

  return menu(path);
}
