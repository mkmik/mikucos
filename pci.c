#include <pci.h>
#include <bios32.h>

#include <pci_names.h>
#include <pci_ids.h>
#include <libc.h>
#include <malloc.h>

LIST_HEAD(pci_devices);

struct bios32_indirect_t pci_indirect = BIOS32_INDIRECT_CONSTRUCTOR;

void pci_init() {
  xprintf("initializing PCI bus\n");
  pci_find_bios();
  pci_scan_all();
}

void pci_find_bios() {
  u_int32_t pcibios_entry = 0;
  
  if(!bios32_indirect.address)
    bios32_find_directory();
  
  if ((pcibios_entry = bios32_service(PCI_SERVICE))) {
    pci_indirect.address = pcibios_entry + PAGE_OFFSET;
    xprintf("PCI: found PCI BIOS at %p\n", pci_indirect.address);
  } else 
    xprintf("PCI: PCI BIOS32 Service not found\n");
  
}


int pci_bios_write(int bus, 
		   int dev, 
		   int fn, 
		   int regi,
		   int len, u_int32_t value) {
  unsigned long result = 0;
  unsigned long bx = ((bus << 8) | (dev << 3) | fn);
  
  if ((bus > 255) || (dev > 31) || (fn > 7) || (regi > 255))
    return -1;

#define BIOS_WRITE(size)             \
    __asm__("lcall *(%%esi); cld\n\t"\
	    "jc 1f\n\t"              \
	    "xor %%ah, %%ah\n"       \
	    "1:"                     \
	    : "=a" (result)          \
	    : "0" (PCIBIOS_WRITE_CONFIG_ ## size), \
	    "c" (value),             \
	    "b" (bx),                \
	    "D" ((long)regi),         \
	    "S" (&pci_indirect));    
  
  switch (len) {
  case 1:
    BIOS_WRITE(BYTE);
    break;
  case 2:
    BIOS_WRITE(WORD);
    break;
  case 4:
    BIOS_WRITE(DWORD);
  }

  return (int)((result & 0xff00) >> 8);
}

int pci_bios_read(int bus, int dev, int fn, int regi, 
		  int len, u_int32_t *value) {
  unsigned long result = 0;
  unsigned long bx = ((bus << 8) | (dev << 3) | fn);
  
  if (!value || (bus > 255) || (dev > 31) || (fn > 7) || (regi > 255))
    return -1;
  
#define BIOS_READ(size)             \
 __asm__("lcall *(%%esi); cld\n\t"  \
	    "jc 1f\n\t"             \
	    "xor %%ah, %%ah\n"      \
	    "1:"                    \
	    : "=c" (*value),        \
	    "=a" (result)           \
	    : "1" (PCIBIOS_READ_CONFIG_ ## size), \
	    "b" (bx),               \
	    "D" ((long)regi),        \
	    "S" (&pci_indirect));   

  switch (len) {
  case 1:
    BIOS_READ(BYTE);
    break;
  case 2:
    BIOS_READ(WORD);
    break;
  case 4:
    BIOS_READ(DWORD);
  }

  return (int)((result & 0xff00) >> 8);
} 
    
int pci_bios_write_byte(struct pci_dev* dev, int regi, u_int8_t value) {
  return pci_bios_write(dev->bus, dev->dev, dev->fn, regi, 1, (u_int32_t)value);
}

int pci_bios_write_word(struct pci_dev* dev, int regi, u_int16_t value) {
  return pci_bios_write(dev->bus, dev->dev, dev->fn, regi, 2, (u_int32_t)value);
}

int pci_bios_write_dword(struct pci_dev* dev, int regi, u_int32_t value) {
  return pci_bios_write(dev->bus, dev->dev, dev->fn, regi, 4, (u_int32_t)value);
}

int pci_bios_read_byte(struct pci_dev* dev, int regi, u_int8_t* value) {
  return pci_bios_read(dev->bus, dev->dev, dev->fn, regi, 1, (u_int32_t*)value);
}

int pci_bios_read_word(struct pci_dev* dev, int regi, u_int16_t* value) {
  return pci_bios_read(dev->bus, dev->dev, dev->fn, regi, 2, (u_int32_t*)value);
}

int pci_bios_read_dword(struct pci_dev* dev, int regi, u_int32_t* value) {
  return pci_bios_read(dev->bus, dev->dev, dev->fn, regi, 4, (u_int32_t*)value);
}

void pci_scan_all() {
  int i;

  u_int32_t val=0;
  u_int16_t command=0;
  u_int32_t rom=0;
  u_int8_t irq=0;
  struct pci_dev device;
 
  for(device.bus=0; device.bus<0x4; device.bus++) { 
    uprintf("Scanning bus %d\n", device.bus);
    
    for(device.dev=0; device.dev<0x20; device.dev++) {
      for(device.fn=0; device.fn<0x8; device.fn++) {
	
	if(pci_bios_read_byte(&device, PCI_HEADER_TYPE, &device.header_type))
	  printf("error reading\n");
	
	if(pci_bios_read_word(&device, PCI_VENDOR_ID, &device.vendor))
	  printf("error reading\n");
	
	if(device.vendor == 0 || device.vendor == 0xFFFF)
	  continue;
	
	if(pci_bios_read_word(&device, PCI_DEVICE_ID, &device.device))
	  printf("error reading\n");
	
	if(pci_bios_read_dword(&device, PCI_CLASS_REVISION, &val))
	  printf("error reading\n");
	device.class = val >> 16;
	device.rev = val & 0xf;
	
	uprintf("PCI: 0:%x.%x %s:", device.dev, device.fn, pci_class_name(device.class));
	pci_name_device(&device);
	
	///     
	pci_bios_read_word(&device, PCI_COMMAND, &command);
	
	uprintf(" ( ");
	if(command & PCI_COMMAND_IO)
	  uprintf("IO ");
	if(command & PCI_COMMAND_MEMORY)
	  uprintf("MEMORY ");
	if(command & PCI_COMMAND_MASTER)
	  uprintf("MASTER ");
	uprintf(")\n");
	
	pci_bios_read_dword(&device, PCI_ROM_ADDRESS, &rom);
	uprintf(" ROM present?: %d, address %p\n", rom & PCI_ROM_ADDRESS_ENABLE, rom);
	
	device.io = device.memory = 0;
	for(i=0; i<6; i++) {
	  pci_bios_read_dword(&device, PCI_BASE_ADDRESS_0 + 4*i, &val);
	  if(!val)
	    continue;
	  
	  uprintf(" base address %d ", i); 
	  if(val & PCI_BASE_ADDRESS_SPACE_IO) {
	    device.io = val & PCI_BASE_ADDRESS_IO_MASK;
	    uprintf("I/O %p\n", device.io);
	  } else {
	    device.memory = val & PCI_BASE_ADDRESS_MEM_MASK;
	    uprintf("Mem %p\n", device.memory);
	  }
	}
	
	device.irq = 0;
	pci_bios_read_byte(&device, PCI_INTERRUPT_LINE, &irq);
	if(irq && (irq != 0xFF)) {
	  uprintf(" irq %d\n", irq);
	  device.irq = irq;
	}

	pci_dev_t* newdev = malloc(sizeof(pci_dev_t));
	memcpy(newdev, &device, sizeof(pci_dev_t));
	list_add_tail(&newdev->list, &pci_devices);
	
      }
    }
  }
}


pci_dev_t* pci_find_device(u_int32_t id, pci_dev_t* last) {
  if(last)
    return 0;

  uprintf("SEARCHING device id %x\n", id);

  list_head_t *pos;
  list_for_each(pos, &pci_devices) {
    pci_dev_t *dev = list_entry(pos, pci_dev_t, list);
    uprintf("scanning device %p id %x\n", dev, dev->id);
    if(dev->id == id)
      return dev;
  }
  return 0;
}
