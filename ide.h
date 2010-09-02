#ifndef _IDE_H
#define _IDE_H

#include <types.h>
#include <device.h>
#include <block.h>

#define IDE_BASE_REG      ide_base_reg(this)
#define IDE_2BASE_REG     (ide_base_reg(this)+0x200)
#define DATA_REG          (IDE_BASE_REG + 0)
#define ERROR_REG         (IDE_BASE_REG + 1)
#define SECTOR_CNT_REG    (IDE_BASE_REG + 2)
#define SECTOR_REG        (IDE_BASE_REG + 3)
#define CYLINDER_LOW_REG  (IDE_BASE_REG + 4)
#define CYLINDER_HIGH_REG (IDE_BASE_REG + 5)
#define DRIVE_HEAD_REG    (IDE_BASE_REG + 6)
#define STATUS_REG        (IDE_BASE_REG + 7)
#define COMMAND_REG       STATUS_REG
#define ALT_STATUS_REG    (IDE_2BASE_REG + 6)
#define CONTROL_REG       ALT_STATUS_REG

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
#define SEC_LEN   512
#define CMD_READ  0x20
#define CMD_WRITE 0x30
#define CMD_IDENT 0xEC

u_int8_t BUFF[512];
u_int32_t sec_buffed;

#define TASKFILE_BASE SECTOR_CNT_REG
struct taskfile {
  u_int8_t sector_cnt;
  u_int8_t sector;
  u_int8_t h_cylinder;
  u_int8_t l_cylinder;
  u_int8_t drive_head;
};

u_int8_t print_status(device_t* this);
void write_sector(device_t* this, const u_int8_t *buffer, u_int32_t sector, u_int16_t num);
void read_sector(device_t* this, u_int8_t *buffer, u_int32_t sector, u_int16_t num);

// structured VFS code

typedef u_int8_t ide_drive_t;

struct ide_private {
  ide_drive_t drive;
  int controller;
  char model[41];
  u_int32_t size;
};
typedef struct ide_private ide_private_t;

void ide_init();
void ide_device_init(device_t* this);

// device ops 

int ide_read_blocks(device_t* this, block_t* buffer, off_t start, size_t len);
int ide_write_blocks(device_t* this, const block_t* buffer, off_t start, size_t len);
int ide_get_device_option(device_t* this, const char* name, va_list args);

// private

int ide_base_reg(device_t* this);

// test

int internal_ide_test();

#endif
