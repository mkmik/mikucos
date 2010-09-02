#include <partition.h>
#include <mbr.h>
#include <malloc.h>
#include <file.h>

#define private(arg) ((partition_private_t*)arg->private)

device_ops_t partition_ops = {
  .devfs_filename = partition_devfs_filename,
  .read_blocks = partition_read_blocks,
  .get_device_option = partition_get_device_option
};

char* partition_devfs_filename(device_t* this) {
  device_t *disk = private(this)->disk;
  char *basedir = invoke(driver, disk->driver, devfs_basedir)(disk->driver);
  basedir = file_path_dropfirst(basedir);
  
  char *devname = invoke(device, disk, devfs_filename)(disk);
  char *name = malloc(strlen(basedir) + strlen(devname) + strlen(this->name) + 3);
  strcpy(name, basedir);
  strcat(name, "/");
  strcat(name, devname);
  strcat(name, "/");
  strcat(name, this->name);
  
  return name;
}

int partition_read_blocks(device_t* this, block_t* buffer, off_t start, size_t len) {
  // checks
  if((start < 0) || ((start+len) >= (private(this)->len)))
    return -1;

  assert(private(this)->disk);

  return invoke(device, private(this)->disk, read_blocks)(private(this)->disk,
							  buffer,
							  private(this)->start + start,
							  len);
}

int partition_get_device_option(device_t* this, const char* name, va_list args) {
  if(strcmp("device_size", name) == 0)
    return (private(this)->start + private(this)->len) * sizeof(block_space_t);
  return -1;
}

static block_space_t block;
void partition_lookup(driver_t* this, device_t *device) {
  // hack to not check partitions inside partitions themselves
  if(this == device->driver)
    return;

  mbr_t *mbr = (mbr_t*)&block;
  
  uprintf("looking up for partitions in device %p\n", device);
  invoke(device, device, read_blocks)(device, block, 0, 1);
  if(mbr->signature != 0xaa55) {
    uprintf("No partition table (invalid mbr signature)\n");
    return;
  }
  
  int i;
  for(i=0; i<4; i++) {
    if((mbr->pe[i].type == 0 ) && (mbr->pe[i].lba_sector == 0))
      continue;
    
    device_t *partition = malloc(sizeof(device_t));
    device_init_device(partition);
    partition->ops = &partition_ops;
    partition->name = malloc(strlen("part") + 3);
    strcpy(partition->name, "part");
    partition->name[strlen("part")] = '0' + i;
    partition->name[strlen("part")+1] = 0;
    partition->private = malloc(sizeof(partition_private_t));
    private(partition)->disk = device;
    private(partition)->start = mbr->pe[i].lba_sector;
    private(partition)->len = mbr->pe[i].lba_len;
    private(partition)->number = i;

    //    device_attach_device(partition, device);
    device_attach_driver(partition, this);
    invoke(device, partition, register_devfs)(partition);
  }
}
