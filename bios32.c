#include <bios32.h>

#include <libc.h>

struct bios32_indirect_t bios32_indirect = BIOS32_INDIRECT_CONSTRUCTOR;

u_int32_t bios32_service(u_int32_t service) {
  u_int8_t return_code;      /* %al */
  u_int32_t address;          /* %ebx */
  u_int32_t length;           /* %ecx */
  u_int32_t entry;            /* %edx */
  u_int32_t flags;

  __save_flags(flags); hal_disable_interrupts();
  asm("lcall *(%%edi); cld"
      : "=a" (return_code),
      "=b" (address),
      "=c" (length),
      "=d" (entry)
      : "0" (service),
      "1" (0),
      "D" (&bios32_indirect));
  __restore_flags(flags);
  
  switch (return_code) {
  case 0:
    return address + entry;
  case 0x80:      /* Not present */
    printf("bios32_service(0x%lx): not present\n", service);
    return 0;
  default: /* Shouldn't happen */
    printf("bios32_service(0x%lx): returned 0x%x -- BIOS bug!\n",
	   service, return_code);
    return 0;
  }
}

int bios32_find_directory(void) {
  union bios32 *check;
  u_int8_t sum;
  int i, length;
  
  /*
   * Follow the standard procedure for locating the BIOS32 Service
   * directory by scanning the permissible address range from
   * 0xe0000 through 0xfffff for a valid BIOS32 structure.
   */
  
  for (check = (union bios32 *)(0xe0000);
       check <= (union bios32 *)(0xffff0);
       ++check) {
    if (check->fields.signature != BIOS32_SIGNATURE)
      continue;
    length = check->fields.length * 16;
    if (!length)
      continue;
    sum = 0;
    for (i = 0; i < length ; ++i)
      sum += check->chars[i];
    if (sum != 0)
      continue;
    if (check->fields.revision != 0) {
      printf("PCI: unsupported BIOS32 revision %d at %p\n",
	     check->fields.revision, check);
      continue;
    }

    if (check->fields.entry >= 0x100000) {
      printf("PCI: BIOS32 entry (0x%p) in high memory, cannot use.\n", check);
      return 0;
    } else {
      u_int32_t bios32_entry = check->fields.entry;
      bios32_indirect.address = bios32_entry + PAGE_OFFSET;
    }
    break;  /* Hopefully more than one BIOS32 cannot happen... */
  }
  
  return 0;
}
