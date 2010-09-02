#include <uart.h>
#include <ide.h>
#include <io.h>
#include <libc.h>
#include <ctype.h>
#include <hal.h>
#include <timer.h>

#include <driver.h>
#include <device.h>
#include <devfs.h>
#include <process.h>

#include <file_api.h>
#include <mbr.h>

#define private(this) ((ide_private_t*)((this)->private))

driver_t ide_driver;
device_ops_t ide_ops = {
  .init = ide_device_init,
  .read_blocks = ide_read_blocks,
  .write_blocks = ide_write_blocks,
  .get_device_option = ide_get_device_option
};

static int use_lba = 1;
static void enable_ien(device_t* this);

u_int8_t print_status(device_t* this) {
  u_int8_t status = (u_int8_t)inb(STATUS_REG);
  xprintf("status: %x (", (u_int32_t)status);
  if(status & STATUS_BUSY)
    xprintf("BUSY ");
  if(status & STATUS_DRDY)
    xprintf("DRDY ");
  if(status & STATUS_DRQ)
    xprintf("DRQ ");
  if(status & STATUS_ERROR)
    xprintf("ERR ");
  xprintf(")\n");

  if(status & STATUS_ERROR) {
    u_int8_t error =  (u_int8_t)inb(ERROR_REG);
      xprintf("error: %x (", (u_int32_t)error);
    if(error & ERROR_IDNF)
      xprintf("IDNF ");
    if(error & ERROR_ABORT)
      xprintf("ABORT ");
      xprintf(")\n");
  }
  return(status);
}

void make_request(struct taskfile* taskfile, u_int8_t drive,
			 u_int32_t sector, u_int16_t num) {
  taskfile->sector_cnt = num == 256 ? 0 : num;
  taskfile->h_cylinder = 0; // hardcode
  taskfile->h_cylinder=0;   // hardcode
  if(use_lba) {
    sector++;
    taskfile->sector = (sector & 0xff);
    taskfile->l_cylinder = (sector & 0xff00)>> 8;
    taskfile->h_cylinder = (sector & 0xff0000) >> 16;
    taskfile->drive_head = 0xA0 | drive << 4;
  } else {
    taskfile->sector = sector & 0xff;
    taskfile->drive_head = 0xE0 | drive << 4;
  }
}

void file_request(device_t* this, struct taskfile* taskfile) {
  enable_ien(this);

  int i;
  char* tf = (char*)taskfile;
  int port = TASKFILE_BASE;
  for(i=0; i<sizeof(*taskfile); i++)
    outb(tf[i], port+i);
}

static void enable_ien(device_t* this) {
    u_int8_t status;
  timetick_t oldtime = timer_ticks;
  do {
    status = (u_int8_t)inb(ALT_STATUS_REG);
    if(timer_ticks - oldtime > 100*HZ/1000) {
      xprintf("ide timeout, waiting BUSY to clear\n");
      return;
    }
  } while(status & STATUS_BUSY);

  outb(0, CONTROL_REG);

  oldtime = timer_ticks;
  do {
    status = (u_int8_t)inb(ALT_STATUS_REG);
    if(timer_ticks - oldtime > 100*HZ/1000) {
      xprintf("ide timeout, waiting DRDY\n");
      return;
    }
  } while(!(status & STATUS_DRDY));
}

void send_command(device_t* this, u_int8_t cmd) {
  outb(cmd, COMMAND_REG);
}
//-----------------------------primo livello-----------------
void write_sector(device_t* this, const u_int8_t* buffer, u_int32_t sector, u_int16_t num){
  struct taskfile taskfile;
  u_int16_t i;
  make_request(&taskfile, private(this)->drive, sector, num);
  file_request(this, &taskfile);
  send_command(this, CMD_WRITE);
  for(i=0; i<256; i++) 
      outw(((u_int16_t*)buffer)[i], DATA_REG);
  
 }
void read_sector(device_t* this, u_int8_t *buffer,u_int32_t sector, u_int16_t num){
  struct taskfile taskfile;
  u_int16_t  i;
  /*  if(sector==sec_buffed){
    memcpy(buffer,BUFF,SEC_LEN);
  } 
  else */ {
    make_request(&taskfile, private(this)->drive, sector, num);
    file_request(this, &taskfile);
    send_command(this, CMD_READ);
    for(i=0; i<256; i++) 
      ((u_int16_t*)buffer)[i]= inw(DATA_REG);
    memcpy(BUFF,buffer,SEC_LEN);
  }
  sec_buffed=sector;
}

static int device_ident(device_t* this) {
  int drive = private(this)->drive;
  struct taskfile taskfile;
  make_request(&taskfile, drive, 0, 0);
  file_request(this, &taskfile);
  send_command(this, CMD_IDENT);

  u_int8_t status;
  timetick_t oldtime = timer_ticks;
  do {
    status = (u_int8_t)inb(ALT_STATUS_REG);
    if(timer_ticks - oldtime > 100*HZ/1000) {
      xprintf("ident timeout!!!\n");
      return 0;
    }
  } while(status & STATUS_BUSY);

  timer_spin_delay(50);
  status = (u_int8_t)inb(STATUS_REG);
  if(!(status & STATUS_DRQ)) {
    xprintf("ident refused!!!\n");
    return 0;
  }

  int i;
  u_int16_t buffer[0x35];

  for(i=0; i<0x1b; i++)
    buffer[i] = inw(DATA_REG);

  for(; i<(0x1b+20); i++) {
    u_int16_t tmp = inw(DATA_REG);
    buffer[i] = (tmp >> 8) | ((tmp & 0xFF) << 8);
  }

  //  for(; i<sizeof(buffer); i++) // unused for now
  //    buffer[i] = inw(DATA_REG);

  memcpy(private(this)->model, &buffer[0x1b], 40);
  private(this)->model[40] = 0;
  
  int size = buffer[1] * buffer[3] * buffer[6] * sizeof(block_space_t);

  private(this)->size = size;

  return 1;
}

int ide_probe(device_t* this) {
  return device_ident(this);
}

void ide_device_init(device_t* this) {
  uprintf("IDE INIT\n");
}

void ide_init() {
  driver_init_driver(&ide_driver);
  ide_driver.name = "IDE";
  driver_register(&ide_driver, "Harddisk");

  int i;
  for(i=0;i<4;i++) {
    device_t *device = malloc(sizeof(device_t));
    assert(device);
    
    device_init_device(device);
    device->name = malloc(4);
    itoa(device->name, 10, i);
    device->ops = &ide_ops;
    ide_private_t *priv = malloc(sizeof(ide_private_t));
    priv->drive = i%2;
    priv->controller = i/2;
    device->private = priv;
    
    if(!ide_probe(device))
      free(device);
    else
      device_attach_driver(device, &ide_driver);
  }
}

int ide_read_blocks(device_t* this, block_t* buffer, off_t start, size_t len) {
  int i;
  for(i=0; i<len; i++) {
    read_sector(this, buffer, start, 1); // use multi read when phasa codes
  }

  return 0;
}

int ide_write_blocks(device_t* this, const block_t* buffer, off_t start, size_t len) {
  int i;
  for(i=0; i<len; i++) {
    write_sector(this, buffer, start, 1); // use multi write when phasa codes
  }

  return 0;
}

int ide_get_device_option(device_t* this, const char* name, va_list args) {
  if(strcmp(name, "device_size") == 0)
    return private(this)->size;
  else if(strcmp(name, "model") == 0) {
    char* buf = va_arg(args, char*);
    int len = va_arg(args, int);
    return strncpy(buf, private(this)->model, len), 0;
  }
  
  return -1;
}

int ide_base_reg(device_t* this) {
  if(private(this)->controller == 0)
    return 0x1f0;
  else if(private(this)->controller == 1)
    return 0x170;
  else
    panicf("IDE controller %d doesn't exist\n");
}


CREATE_ISR_DISPATCHER(ide);
void ide_cisr() {
  uprintf("GOT IDE INTERRUPT");
}

static block_space_t block;
int internal_ide_test(int argc, char** argv) {
  const char *path = "/Devices/Harddisk/IDE/0/part0";
  //  const char *path = "/Devices/Harddisk/IDE/0";

  if(argc > 1) {
    path = argv[1];
  }

  int fd = open(path, 0);
  if(fd == -1) {
    printf("cannot open %s\n", path);
    return 1;
  }

  device_t* device = devfs_device_from_file(process_current()->fd[fd]);
  if(!device) {
    printf("%s is not a device\n", path);
    close(fd);
  }

  // interrupt test
  set_isr(0x20 + 14, ide_isr);
  enable_irq(14);

  int size = get_device_option(fd, "device_size");
  int human = size;
  char *suffix[] = {"", "k", "M", "G", "T", 0};
  int suffix_idx = 0;
  while(human >= 1024) {
    human /= 1024;
    suffix_idx++;
    if(suffix[suffix_idx] == 0)
      break;
  }
  printf("\nSize: %d %sB, %d bytes\n", human, suffix[suffix_idx], size);

  int res;
  memset(block, 0, sizeof(block));

  res = invoke(device, device, read_blocks)(device, (block_t*)block, 0, 1);

  hexdump(block, 128);
  //  printf("...\n");
  //  hexdump(((char*)block)+512-128, 128);

  printf("res = %d\n", res);

  close(fd);
  

  return 0;
}
