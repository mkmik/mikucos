#include <multiboot.h>
#include <libc.h>

u_int32_t mb_magic;
struct multiboot_info *mb_info;

#define PAD "                                         "
#define noprintf(args...)

void multiboot_print() {
  int row = getRow();
  int col = getCol();
  
  noprintf("multiboot options\n");
  if(mb_magic == MULTIBOOT_BOOTLOADER_MAGIC) {
    if(mb_info->flags & MULTIBOOT_HAS_LOADER_NAME)
      noprintf(PAD "loader: %s\n", mb_info->loader_name);
    
    if(mb_info->flags & MULTIBOOT_HAS_CMDLINE)
      noprintf(PAD "cmdline: %s\n", mb_info->cmdline);
    
    if(mb_info->flags & MULTIBOOT_HAS_VBE)
      uprintf(PAD "vbe installed\n" );
    
    if(mb_info->flags & MULTIBOOT_HAS_MODS ) {
      int i;
      noprintf(PAD "%d module(s) present\n", mb_info->mods_count);
      uprintf("multiboot: %d module(s) present\n", mb_info->mods_count);
      for(i=0; i< mb_info->mods_count; i++) {
	uprintf(" %d %x\n",i, (mb_info->mods_addr[i].mod_start));
/*	noprintf(PAD " contents:");
	s= (char*)mb_info->mods_addr[i].mod_start;
	while(s <(char*)mb_info->mods_addr[i].mod_end || s)
	  putchar(*s++); 
	putchar('\n');*/
      }
    }
    
    if(mb_info->flags & MULTIBOOT_HAS_MEMORY)
      noprintf(PAD "memory info: low %d, high %d kb\n",
	     mb_info->mem_lower, mb_info->mem_upper);
    
    if(mb_info->flags & MULTIBOOT_HAS_MEM_MAP) {
      struct multiboot_address_range* buffer= mb_info->mmap_addr;
      noprintf(PAD "memory map present \n");

      while(((char*)buffer - (char*)mb_info->mmap_addr) < mb_info->mmap_length) {
	noprintf(PAD " %x %x type %s\n",
	       buffer->base_addr_low,
	       buffer->length_low,
	       buffer->type  == 1 ? "ram" : "hw" );
	buffer = (struct multiboot_address_range*)
	  (((char*)buffer)+buffer->size+4);
      }
    }

    if(mb_info->flags & MULTIBOOT_HAS_ELF_SHDR) {
      noprintf(PAD "ELF symbols found\n");
    }

    if(mb_info->flags & MULTIBOOT_HAS_MODS) {
      noprintf(PAD "Found %d modules\n", mb_info->mods_count);
    }
    
    if(mb_info->flags & MULTIBOOT_HAS_BIOS_CONFIG) {
      u_int8_t* table = mb_info->config_table;
      noprintf(PAD "bios config table: \n");
      noprintf(PAD " model %x submodel %x\n", table[1], table[2]);
      noprintf(PAD " extended BIOS allocated %d\n", (table[5] & (1<<2)) != 0);
      noprintf(PAD " RTS present %d\n", (table[5] & (1<<5)) != 0);
      noprintf(PAD " has 2nd 8259 %d\n", (table[5] & (1<<6)) != 0);

      uprintf("BIOS model %x submodel %x\n", table[1], table[2]);
    }
  } else
    xprintf(": notfrom multiboot!\n");

  setPos(row, col); //restore position
}

const char* multiboot_cmdline() {
  if(mb_magic == MULTIBOOT_BOOTLOADER_MAGIC) 
    return mb_info->cmdline;
  else
    return "";
}

const char* multiboot_loader_name() {
  if(mb_magic == MULTIBOOT_BOOTLOADER_MAGIC)
    return mb_info->loader_name;
  else
    return "";
  
}

void* multiboot_get_module(int i) {
  if(i < mb_info->mods_count)
    return (void*)mb_info->mods_addr[i].mod_start;
  else 
    return 0;
}

int multiboot_get_module_length(int i) {
  if(i < mb_info->mods_count)
    return mb_info->mods_addr[i].mod_end - mb_info->mods_addr[i].mod_start;
  else
    return 0;
}
