#include <tar.h>
#include <types.h>
#include <libc.h>

int tar_size(struct tar_header* header) {
  char *size = header->size;
  int i=11, s=0;
  while(i--)
    s = s*8 + (*size++-'0') ;
  return s;
}

int tar_file_size(tar_archive_t* file) {
  return tar_size(&file->block[0].header);
}

char* tar_file_name(tar_archive_t* file) {
  return file->block[0].header.name;
}

int tar_blocks_from_size(int size) {
  int blocks = size / BLOCKSIZE;
  if((blocks * BLOCKSIZE) < size)
    blocks++;
  return blocks;
}

int tar_file_blocks(tar_archive_t* file) {
  return tar_blocks_from_size(tar_size(&file->block[0].header));
}

void *tar_file_data(tar_archive_t* file) {
  return (void*)&file->block[1];
}

tar_archive_t* tar_next_file(tar_archive_t* arch) {
  return (tar_archive_t*)&arch->block[tar_file_blocks(arch)+1];
}

int tar_is_valid_file(tar_archive_t* file) {
  return strncmp(file->block[0].header.magic, "ustar", 5) == 0;
}

tar_archive_t* tar_find_file(tar_archive_t* arch, char* name) {
  while(tar_is_valid_file(arch)) {
    if(strcmp(arch->block[0].header.name, name) == 0)
      return arch;
    arch = tar_next_file(arch);
  }
  return 0;
}

static void print_file(tar_archive_t* file) {
  int size,i;
  struct tar_header *header = &file->block[0].header;
  char *data;
  
  size = tar_size(header);
  uprintf("name: %s\n", header->name);
  uprintf("size: %d\n", size);
  uprintf("magic: %s\n", header->magic);
  uprintf("type: %c\n", header->typeflag);  
  data = file->block[1].buffer;
  uprintf("<BEGIN>\n");
  i=size;
  while(i--)
    uputchar(*data++);
  uprintf("<END>\n");
}

void tar_test(tar_archive_t* arch, char *name) {
  tar_archive_t *oldarch;
    
  oldarch = arch;
  if(!arch) {
    uprintf("invalid tar archive\n");
    return;
  }
  
  uprintf("reading tar from %p...\n", arch);
  while(tar_is_valid_file(arch)) {
    uprintf(" %s", arch->block[0].header.name);
    if(arch->block[0].header.typeflag == DIRTYPE)
      uprintf(" (DIR)");
    uprintf("\n");
    arch = tar_next_file(arch);
  }
  uprintf("done...\n\n");

  arch = tar_find_file(oldarch, name);
  if(arch)
    print_file(arch);
  else
    uprintf("no such file '%s'\n", name);  
}
