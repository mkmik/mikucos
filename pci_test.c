#include <pci_test.h>
#include <uart.h>
#include <libc.h>
#include <hal.h>
#include <init.h>
#include <timer.h>

#include <pci.h>
#include <pci_names.h>
#include <pci_ids.h>
#include <lance_driver.h>

void pci_test_init() {
  pci_init();
}

void pci_test_run() {
  int i;

  u_int32_t val=0;
  u_int16_t command=0;
  u_int32_t rom=0;
  u_int8_t irq=0;
  struct pci_dev device;
  struct pci_dev lance = {-1, };
 
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
      
      printf("PCI: 0:%x.%x %s:", device.dev, device.fn, pci_class_name(device.class));
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

      if(device.vendor == PCI_VENDOR_ID_AMD &&
	 device.device == PCI_DEVICE_ID_AMD_LANCE)
	lance = device;	
    }
  }
  }
  if(lance.bus != -1) {
    lance_driver(&lance);
  }
}
