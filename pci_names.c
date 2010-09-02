/*
 *	PCI Class and Device Name Tables
 *
 *	Copyright 1993--1999 Drew Eckhardt, Frederic Potter,
 *	David Mosberger-Tang, Martin Mares
 */

#include <types.h>
#include <pci.h>
#include <libc.h>
#include <uart.h>

struct pci_device_info {
  unsigned short device;
  unsigned short seen;
  const char *name;
};

struct pci_vendor_info {
  unsigned short vendor;
  unsigned short nr;
  const char *name;
  struct pci_device_info *devices;
};

/*
 * This is ridiculous, but we want the strings in
 * the .init section so that they don't take up
 * real memory.. Parse the same file multiple times
 * to get all the info.
 */
#define VENDOR( vendor, name )		static char __vendorstr_##vendor[]  = name;
#define ENDVENDOR()
#define DEVICE( vendor, device, name ) 	static char __devicestr_##vendor##device[]  = name;
#include "pci_devlist.h"


#define VENDOR( vendor, name )		static struct pci_device_info __devices_##vendor[]  = {
#define ENDVENDOR()			};
#define DEVICE( vendor, device, name )	{ 0x##device, 0, __devicestr_##vendor##device },
#include "pci_devlist.h"

  static struct pci_vendor_info  pci_vendor_list[] = {
#define VENDOR( vendor, name )		{ 0x##vendor, sizeof(__devices_##vendor) / sizeof(struct pci_device_info), __vendorstr_##vendor, __devices_##vendor },
#define ENDVENDOR()
#define DEVICE( vendor, device, name )
#include "pci_devlist.h"
  };

#define VENDORS (sizeof(pci_vendor_list)/sizeof(struct pci_vendor_info))

void  pci_name_device(struct pci_dev *dev) {
  const struct pci_vendor_info *vendor_p = pci_vendor_list;
  int i = VENDORS;

  do {
    if (vendor_p->vendor == dev->vendor)
      goto match_vendor;
    vendor_p++;
  } while (--i);

  /* Couldn't find either the vendor nor the device */
  //  printf("PCI device %x:%x\n", dev->vendor, dev->device);
  uprintf("PCI device %x:%x\n", dev->vendor, dev->device);
  return;

 match_vendor: {
    struct pci_device_info *device_p = vendor_p->devices;
    int i = vendor_p->nr;

    while (i > 0) {
      if (device_p->device == dev->device)
	goto match_device;
      device_p++;
      i--;
    }

    /* Ok, found the vendor, but unknown device */
    //    printf("PCI device %x:%x (%s)\n", dev->vendor, dev->device, vendor_p->name);
    uprintf("PCI device %x:%x (%s)\n", dev->vendor, dev->device, vendor_p->name);
    return;

    /* Full match */
 match_device: {
      //      printf("%s %s:\n", vendor_p->name, device_p->name);
      uprintf("%s %s:\n", vendor_p->name, device_p->name);
    }
  }
}

/*
 *  Class names. Not in .init section as they are needed in runtime.
 */

static u_int16_t pci_class_numbers[] = {
#define CLASS(x,y) 0x##x,
#include "pci_classlist.h"
};

static char *pci_class_names[] = {
#define CLASS(x,y) y,
#include "pci_classlist.h"
};

char * pci_class_name(u_int32_t class) {
  int i;
  
  for(i=0; i<sizeof(pci_class_numbers)/sizeof(pci_class_numbers[0]); i++)
    if (pci_class_numbers[i] == class)
      return pci_class_names[i];
  return 0;
}


