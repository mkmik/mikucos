#include <fat.h>
#include <fat_low.h>
#include <fat_private.h>
#include <filesystem_implementation.h>

#include <stdio.h>
#include <malloc.h>

static struct file_ops fat_ops = {
  //  .read = fat_read,
  //  .write = fat_write,
  .read_dir = fat_read_dir
};

static struct filesystem_ops fat_fs_ops = {
  .init = fat_fs_init
};

static filesystem_t fat = {
  .name = "fat",
  .ops = &fat_ops,
  .fops = &fat_fs_ops
};

void fat_init() {
  filesystem_register(&fat);
}

static block_space_t rootdir_buffer;
int fat_read_dir(file_t* this, char* buffer, size_t len, int* flags) {
  assert(len > 8);

  int dir_len = 512;
  const int page_entries = 16;

  fat_dir_entry_t *dir = (fat_dir_entry_t*)rootdir_buffer;
  int entry, page, p = this->position++;
  int n;
  char *ext;

  if(this->fnode->file_id != 0)
    return -1;

  for(page=0; page < (dir_len/page_entries); page++) {
    filesystem_read_blocks(physical(this->fnode->mounted),
			   rootdir_buffer,
			   private(this->fnode->mounted)->rootdir_start + page,
			   1);    
    
    for(entry=0; entry<page_entries; entry++) {
      
      if(dir[entry].name[0] == 0)
	return -1;
      if(dir[entry].hidden || dir[entry].volume_label)
	continue;
      if(!p) {
	strncpy(buffer, dir[entry].name, 8);
	for(n=0; n<8; n++)
	  if(buffer[n] == ' ')
	    buffer[n] = 0;
	buffer[8] = 0;

	if(dir[entry].ext[0] != ' ') {
	  strcat(buffer, ".");

	  ext = buffer + strlen(buffer);
	  strncpy(ext, dir[entry].ext, 3);
	  for(n=0; n<3; n++) 
	    if(ext[n] == ' ')
	      ext[n] = 0;
	  ext[3] = 0;	  	  
	}

	/*	uprintf("file '%s', subdir %d, start %d, len %d bytes\n",
		buffer,
		dir[entry].subdirectory,
		dir[entry].first_cluster,
		dir[entry].length
		); */

	if(dir[entry].subdirectory)
	  *flags = DIRENT_DIR;
	else 
	  *flags = DIRENT_FILE;
	return dir[entry].first_cluster; // file_id = first_cluster ... discussion...
      }
      p--;
    }
  }

  return -1;
}

static block_space_t boot_block;
int fat_fs_init(mounted_filesystem_t* this) {
  fat16_bootsector_t *boot = (fat16_bootsector_t*)boot_block;

  xprintf("trying to mount fat\n");
  
  this->private = calloc(sizeof(fat_private_t), 1);

  invoke(device, physical(this)->device, read_blocks)(physical(this)->device, boot_block, 0, 1);

#define prints(field) xprintf(# field " '%s' (offset %x)\n",boot->field,&((typeof(boot))0)->field)
#define printd(field) xprintf(# field " %d (offset %x)\n",boot->field,&((typeof(boot))0)->field)
#define printx(field) xprintf(# field " %x (offset %x)\n",boot->field,&((typeof(boot))0)->field)
  prints(OEM);
  printd(bytes_per_sector);
  printd(sectors_per_cluster);
  printd(reserved_sector);
  printd(number_of_fats);
  printd(root_dir_entries);
  printd(logical_sectors);
  printx(medium_descriptor);
  printd(sectors_per_fat);
  printd(sectors_per_track);
  printd(number_heads);
  printd(number_hidden_sectors);
  printd(big_logical_sectors);
  printd(logical_drive_number);
  printx(extended_signature);
  printx(serial_number);
  //  prints(label);
  //  prints(type);

  if(boot->bytes_per_sector != 512) {
    xprintf("sectorsize != 512, not implemented\n");
    return 0;
  }

  if(boot->root_dir_entries == 0) {
    printf("not implementing FAT32\n");
    return 0;
  }

  private(this)->reserved_sector = boot->reserved_sector;
  private(this)->sectors_per_fat = boot->sectors_per_fat;
  private(this)->number_hidden_sectors = boot->number_hidden_sectors;
  private(this)->number_of_fats = boot->number_of_fats;
  private(this)->root_dir_entries = boot->root_dir_entries;

  private(this)->fat_start = boot->reserved_sector + boot->number_hidden_sectors;

  private(this)->rootdir_start = boot->reserved_sector + boot->number_hidden_sectors + 
    boot->number_of_fats * boot->sectors_per_fat;

  private(this)->rootdir_sectors = (boot->root_dir_entries*32 + boot->bytes_per_sector - 1) / 
    boot->bytes_per_sector;
  

  physical(this)->cluster_size = boot->sectors_per_cluster;

  physical(this)->clustered_data_start = private(this)->rootdir_start + 
    private(this)->rootdir_sectors - 2*boot->sectors_per_cluster;

  private(this)->current_page = -1;
  private(this)->dir_page_starting_position = -1;
  private(this)->dir_page_file_id = -1;

  /*  fcluster_t *cluster = calloc(physical(this)->cluster_size, sizeof(block_space_t));
      assert(cluster); */
  
  block_t *buffer = calloc(1, sizeof(block_space_t));
  
  filesystem_read_blocks(physical(this), buffer, private(this)->fat_start, 1);

  hexdump(buffer, 127);

  int i;
  for(i=0; i<10; i++) {
    xprintf("%x ", fat16_read_entry(physical(this), i));
  }
  xprintf("\n");

  filesystem_read_blocks(physical(this), buffer, private(this)->rootdir_start+1, 1);

  hexdump(buffer, 32*14);

  return 1;
}
