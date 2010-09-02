#include <ide_test.h>
#include <libc.h>
#include <io.h>
#include <types.h>
#include <timer.h>
#include <hal.h>

#define IDE_BASE_REG      0x1f0
#define DATA_REG          (IDE_BASE_REG + 0)
#define ERROR_REG         (IDE_BASE_REG + 1)
#define SECTOR_CNT_REG    (IDE_BASE_REG + 2)
#define SECTOR_REG        (IDE_BASE_REG + 3)
#define CYLINDER_LOW_REG  (IDE_BASE_REG + 4)
#define CYLINDER_HIGH_REG (IDE_BASE_REG + 5)
#define DRIVE_HEAD_REG    (IDE_BASE_REG + 6)
#define STATUS_REG        (IDE_BASE_REG + 7)
#define COMMAND_REG       STATUS_REG

#define STATUS_BUSY  (1<<7)
#define STATUS_DRDY  (1<<6)
#define STATUS_DRQ   (1<<3)
#define STATUS_ERROR (1<<0)

#define ERROR_UNC    (1<<6)
#define ERROR_IDNF   (1<<4)
#define ERROR_MCR    (1<<3)
#define ERROR_ABORT  (1<<2)
#define ERROR_TK0NF  (1<<1)
#define ERROR_AMNF   (1<<0)

#define CMD_READ 0x20
#define CMD_WRITE 0x30

static int use_lba = 0;

#define TASKFILE_BASE SECTOR_CNT_REG
struct taskfile {
  u_int8_t sector_cnt;
  u_int8_t sector;
  u_int16_t cylinder; // little endian!
  u_int8_t drive_head;
};

static void make_request(struct taskfile* taskfile, int drive,
			 int sector, int num) {
  taskfile->sector_cnt = num == 256 ? 0 : num;
  taskfile->cylinder = 0; // hardcode
  if(use_lba) {
    taskfile->sector = sector + 1;
    taskfile->drive_head = 0xA0 | drive << 4;
  } else {
    taskfile->sector = sector;
    taskfile->drive_head = 0xE0 | drive << 4;
  }
}

static void file_request(struct taskfile* taskfile) {
  int i;
  char* tf = (char*)taskfile;
  int port = TASKFILE_BASE;
  for(i=0; i<sizeof(*taskfile); i++)
    outb(tf[i], port+i);
}

static void send_command(int cmd) {
  outb(cmd, COMMAND_REG);
}

static void print_status() {
  u_int8_t status = (u_int8_t)inb(STATUS_REG);
  printf("status: %x (", (u_int32_t)status);
  if(status & STATUS_BUSY)
    printf("BUSY ");
  if(status & STATUS_DRDY)
    printf("DRDY ");
  if(status & STATUS_DRQ)
    printf("DRQ ");
  if(status & STATUS_ERROR)
    printf("ERR ");
  printf(")\n");

  if(status & STATUS_ERROR) {
    u_int8_t error =  (u_int8_t)inb(ERROR_REG);
    printf("error: %x (", (u_int32_t)error);
    if(error & ERROR_IDNF)
      printf("IDNF ");
    if(error & ERROR_ABORT)
      printf("ABORT ");
    printf(")\n");
  }
}

void ide_test_init() {
}

char sector_buffer[512];

void ide_test_run() {
  printf("running ide test\n");
  hal_enable_interrupts();

  struct taskfile taskfile;
  make_request(&taskfile, 0, 0, 1);
  
  strcpy(sector_buffer, "ciao questa e' una prova 2");
  
  print_status();
  
  file_request(&taskfile);
  send_command(CMD_WRITE);
  print_status();
  
  int i;
  for(i=0; i<256; i++)
      outw(((u_int16_t*)sector_buffer)[i], DATA_REG);
  print_status();
  delay(200);
  print_status();

}
