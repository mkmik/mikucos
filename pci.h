#ifndef __PCI_H__
#define __PCI_H__

#include <types.h>
#include <avl.h>
#include <list.h>

/* PCI signature: "PCI " */
#define PCI_SIGNATURE           (('P' << 0) + ('C' << 8) + ('I' << 16) + (' ' << 24))

/* PCI service signature: "$PCI" */
#define PCI_SERVICE             (('$' << 0) + ('P' << 8) + ('C' << 16) + ('I' << 24))

/** bios functions */
#define PCIBIOS_PCI_FUNCTION_ID         0xb1XX
#define PCIBIOS_PCI_BIOS_PRESENT        0xb101
#define PCIBIOS_FIND_PCI_DEVICE         0xb102
#define PCIBIOS_FIND_PCI_CLASS_CODE     0xb103
#define PCIBIOS_GENERATE_SPECIAL_CYCLE  0xb106
#define PCIBIOS_READ_CONFIG_BYTE        0xb108
#define PCIBIOS_READ_CONFIG_WORD        0xb109
#define PCIBIOS_READ_CONFIG_DWORD       0xb10a
#define PCIBIOS_WRITE_CONFIG_BYTE       0xb10b
#define PCIBIOS_WRITE_CONFIG_WORD       0xb10c
#define PCIBIOS_WRITE_CONFIG_DWORD      0xb10d
#define PCIBIOS_GET_ROUTING_OPTIONS     0xb10e
#define PCIBIOS_SET_PCI_HW_INT          0xb10f

/** PCI registers */

#define PCI_VENDOR_ID           0x00    /* 16 bits */
#define PCI_DEVICE_ID           0x02    /* 16 bits */
#define PCI_COMMAND             0x04    /* 16 bits */
#define  PCI_COMMAND_IO         0x1     /* Enable response in I/O space */
#define  PCI_COMMAND_MEMORY     0x2     /* Enable response in Memory space */
#define  PCI_COMMAND_MASTER     0x4     /* Enable bus mastering */

#define PCI_CLASS_REVISION      0x08    /* High 24 bits are class, low 8 revision */

#define PCI_HEADER_TYPE         0x0e    /* 8 bits */
#define  PCI_HEADER_TYPE_NORMAL 0
#define  PCI_HEADER_TYPE_BRIDGE 1
#define  PCI_HEADER_TYPE_CARDBUS 2

#define PCI_BASE_ADDRESS_0      0x10    /* 32 bits */
#define PCI_BASE_ADDRESS_1      0x14    /* 32 bits [htype 0,1 only] */
#define PCI_BASE_ADDRESS_2      0x18    /* 32 bits [htype 0 only] */
#define PCI_BASE_ADDRESS_3      0x1c    /* 32 bits */
#define PCI_BASE_ADDRESS_4      0x20    /* 32 bits */
#define PCI_BASE_ADDRESS_5      0x24    /* 32 bits */
#define  PCI_BASE_ADDRESS_SPACE 0x01    /* 0 = memory, 1 = I/O */
#define  PCI_BASE_ADDRESS_SPACE_IO 0x01
#define  PCI_BASE_ADDRESS_SPACE_MEMORY 0x00
#define  PCI_BASE_ADDRESS_MEM_TYPE_MASK 0x06
#define  PCI_BASE_ADDRESS_MEM_TYPE_32   0x00    /* 32 bit address */
#define  PCI_BASE_ADDRESS_MEM_TYPE_1M   0x02    /* Below 1M [obsolete] */
#define  PCI_BASE_ADDRESS_MEM_TYPE_64   0x04    /* 64 bit address */
#define  PCI_BASE_ADDRESS_MEM_PREFETCH  0x08    /* prefetchable? */
#define  PCI_BASE_ADDRESS_MEM_MASK      (~0x0fUL)
#define  PCI_BASE_ADDRESS_IO_MASK       (~0x03UL)

/* Header type 0 (normal device) */
#define PCI_ROM_ADDRESS         0x30    /* Bits 31..11 are address, 10..1 reserved */
#define  PCI_ROM_ADDRESS_ENABLE 0x01
#define PCI_ROM_ADDRESS_MASK    (~0x7ffUL)
/* 0x35-0x3b are reserved */
#define PCI_INTERRUPT_LINE      0x3c    /* 8 bits */
#define PCI_INTERRUPT_PIN       0x3d    /* 8 bits */
#define PCI_MIN_GNT             0x3e    /* 8 bits */
#define PCI_MAX_LAT             0x3f    /* 8 bits */

#define PCI_ID(vendor, device) ((device << 16) | vendor)

/** Every PCI device in the system has a pci_dev entry */
struct pci_dev {
  int bus;
  int dev;
  int fn;

  u_int8_t header_type;
  u_int16_t class;
  union {
    u_int32_t id;
    struct {
      u_int16_t vendor;
      u_int16_t device;
    };
  };
  u_int8_t rev;

  list_head_t list;
  avl_node_t id_tree;

  /* tmp hack */
  u_int32_t memory;
  u_int16_t io;
  u_int8_t irq;
};
typedef struct pci_dev pci_dev_t;


/** initializes PCI bus */
void pci_init();

/** scan */
void pci_scan_all();

/** returns the device if found or null.
 * if there are more devices the one after 'last' is returned.
 * if 'last' is null then the first is returned. */
pci_dev_t* pci_find_device(u_int32_t id, pci_dev_t* last);

/** sets up pci bios entry point */
void pci_find_bios();

/** write into PCI configuration space */
int pci_bios_write(int bus, int dev, int fn, int reg, 
		   int len, u_int32_t value);
/** read from PCI configuration space */
int pci_bios_read(int bus, int dev, int fn, int reg, 
		  int len, u_int32_t *value);

// commodity functions. same as above
int pci_bios_write_byte(struct pci_dev* dev, int reg, u_int8_t value);
int pci_bios_write_word(struct pci_dev* dev, int reg, u_int16_t value);
int pci_bios_write_dword(struct pci_dev* dev, int reg, u_int32_t value);

int pci_bios_read_byte(struct pci_dev* dev, int reg, u_int8_t* value);
int pci_bios_read_word(struct pci_dev* dev, int reg, u_int16_t* value);
int pci_bios_read_dword(struct pci_dev* dev, int reg, u_int32_t* value);

extern struct bios32_indirect_t pci_indirect;

#endif
