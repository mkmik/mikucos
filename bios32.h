#ifndef __BIOS32_H__
#define __BIOS32_H__

#include <init.h>

/* BIOS32 signature: "_32_" */
#define BIOS32_SIGNATURE        (('_' << 0) + ('3' << 8) + ('2' << 16) + ('_' << 24))

/** entry point to protected mode bios service .
 * bios calls are far calls. */

struct bios32_indirect_t {
  u_int32_t address;
  u_int16_t segment;
};

#define BIOS32_INDIRECT_CONSTRUCTOR { 0, __KERNEL_CS }

/*
 * This is the standard structure used to identify the entry point
 * to the BIOS32 Service Directory, as documented in
 *      Standard BIOS 32-bit Service Directory Proposal
 *      Revision 0.4 May 24, 1993
 *      Phoenix Technologies Ltd.
 *      Norwood, MA
 * and the PCI BIOS specification.
 */

union bios32 {
  struct {
    u_int32_t signature;        /* _32_ */
    u_int32_t entry;            /* 32 bit physical address */
    u_int8_t revision;         /* Revision level, 0 */
    u_int8_t length;           /* Length in paragraphs should be 01 */
    u_int8_t checksum;         /* All bytes must add up to zero */
    u_int8_t reserved[5];      /* Must be zero */
  } fields;
  char chars[16];
};


/** entry point of protected mode bios directory */
extern struct bios32_indirect_t bios32_indirect;

/** returns the physical entry point of a protected mode BIOS service */
u_int32_t bios32_service(u_int32_t service);

/** sets up the physical entry point of a protected mode BIOS service */
int bios32_find_directory(void);

#endif
