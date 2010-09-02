#ifndef __MULTIBOOT_H__
#define __MULTIBOOT_H__

#include <types.h>

#define MULTIBOOT_HEADER_MAGIC          0x1BADB002
#define MULTIBOOT_BOOTLOADER_MAGIC      0x2BADB002

#define MULTIBOOT_PAGE_ALIGN    0x00000001
#define MULTIBOOT_MEMORY_INFO   0x00000002
#define MULTIBOOT_VBE_INFO      0x00000004

#define MULTIBOOT_HAS_MEMORY        (1L<<0)
#define MULTIBOOT_HAS_BOOT_HAS_DEVICE   (1L<<1)
#define MULTIBOOT_HAS_CMDLINE       (1L<<2)
#define MULTIBOOT_HAS_MODS          (1L<<3)
#define MULTIBOOT_HAS_AOUT_SYMS     (1L<<4)
#define MULTIBOOT_HAS_ELF_SHDR      (1L<<5)
#define MULTIBOOT_HAS_MEM_MAP       (1L<<6)
#define MULTIBOOT_HAS_BIOS_CONFIG   (1L<<8)
#define MULTIBOOT_HAS_LOADER_NAME   (1L<<9)
#define MULTIBOOT_HAS_VBE           (1L<<11)

struct multiboot_module;
struct multiboot_address_range;

struct multiboot_header {
   u_int32_t magic;
   u_int32_t flags;
   u_int32_t checksum;
   u_int32_t header_addr;
   u_int32_t load_addr;
   u_int32_t load_end_addr;
   u_int32_t bss_end_addr;
   u_int32_t entry_addr;
   u_int32_t mode_type; // 0 = graphics mode, 1 = text mode
   u_int32_t width;
   u_int32_t height;
   u_int32_t depth;
};

struct multiboot_info {
  u_int32_t flags;
  u_int32_t mem_lower;
  u_int32_t mem_upper;
  u_int8_t boot_device[4];
  char *cmdline;
  u_int32_t mods_count;
  struct multiboot_module *mods_addr;
  struct {
    u_int32_t num;
    u_int32_t size;
    u_int32_t addr;
    u_int32_t shndx;
  } syms;
  u_int32_t mmap_length; // total size of buffer
  struct multiboot_address_range *mmap_addr;
  u_int32_t drives_length;
  u_int32_t drives_addr;
  u_int8_t* config_table;
  char *loader_name;
  u_int32_t apm_table;
  /* Video */
  u_int32_t vbe_control_info;
  u_int32_t vbe_mode_info;
  u_int16_t vbe_mode;
  u_int16_t vbe_interface_seg;
  u_int16_t vbe_interface_off;
  u_int16_t vbe_interface_len;
};

struct multiboot_module
{
  /* Physical start and end addresses of the module data itself.  */
  u_int32_t mod_start;
  u_int32_t mod_end;
  
  /* Arbitrary ASCII string associated with the module.  */
  char* string;
  
  /* Boot loader must set to 0; OS must ignore.  */
  u_int32_t reserved;
};

struct multiboot_address_range
{
  u_int32_t size;
  u_int32_t base_addr_low;
  u_int32_t base_addr_high;
  u_int32_t length_low;
  u_int32_t length_high;
  u_int32_t type;
};


/** debugging. prints out the multiboot parameters */
extern void multiboot_print();

/** returns the kernel commandline or an empty string 
 * if not multiboot compatible loader. */
extern const char* multiboot_cmdline();

/** returns the loader name or an empty string
 * if not multiboot compatible loader. */
extern const char* multiboot_loader_name();

/** returns the pointer to the start of the idx'th module */
extern void* multiboot_get_module(int idx);

/** returns the length of the idx'th module */
extern int multiboot_get_module_length(int idx);

extern u_int32_t mb_magic;
extern struct multiboot_info *mb_info;

#endif
