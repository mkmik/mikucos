#include <multiboot.h>

struct multiboot_header mbhead __attribute__((section(".text"))) = {
  .magic = 0x1BADB002,
  .flags = MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO, // flags
  .checksum = 0xE4524FFB, // checksum
  .mode_type = 0, // graphics
  .width = 1024,
  .height = 768,
  .depth = 32
};
