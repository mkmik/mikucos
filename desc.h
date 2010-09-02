#ifndef _PLATFORM_MAL_DESC_H_
#define _PLATFORM_MAL_DESC_H_

// from acaros

#include <types.h>

/** kernel code segment selector */
#define __KERNEL_CS     0x10
/** kernel data segment selector */
#define __KERNEL_DS     0x18

/** user code segment selector */
#define __USER_CS       0x23
/** user data segment selector */
#define __USER_DS       0x2B

/** global task state segment */
#define __TSS           0x40



/** gate (interrupt,trap,call, ...) descriptor.
 * contains the segment and offset
 * of the gate routine
 * and flags */
typedef struct {
  u_int64_t offset_lo:16; // target offset (bits 0..15)
  u_int64_t selector:16;  // target segment selector
  u_int64_t :8;
  u_int64_t type:5;       // gate type \see descrtype */
  u_int64_t dpl:2;        // requested priviledge level to access
  u_int64_t present:1;
  u_int64_t offset_hi:16; // target offset (bits 16..31)
} PACKED mal_gate_t;

/** segment descriptor */
typedef struct {
  u_int64_t limit_lo:16 PACKED;  // segment size (bits 0..15)
  u_int64_t base_lo:16 PACKED;   // base address (bits 0..23)
  u_int64_t base_me:8 PACKED;
  u_int64_t type:5 PACKED;       // segment type \see ia32_descrtype
  u_int64_t dpl:2 PACKED;        // object priviledge level
  u_int64_t present:1 PACKED;    // is the segment present in memory ?
  u_int64_t limit_hi:4 PACKED;   // segment size (bits 16..19)
  u_int64_t flags:4 PACKED;      // miscellaneous flags \see ia32_descrflags 
  u_int64_t base_hi:8 PACKED;    // base address (bits 24..31)
} mal_segment_t;

/** generic descriptor */
typedef union {
  mal_gate_t gate;
  mal_segment_t segment;
  struct {
    u_int64_t:32;
    u_int64_t:8;
    u_int64_t type:5;
    u_int64_t dpl:2;
    u_int64_t present:1;
  };
  u_int64_t raw;
} mal_desc_t;


/** descriptor type */
typedef enum {
  TYPE_FREE=0x00, // free entry (not intel standard)
  TYPE_CODE=0x18, // code segment
  TYPE_DATA=0x10, // datas segment
  TYPE_LDT=0x02,  // local descriptor table
  TYPE_TASKGATE=0x05, // task gate 32 bits
  TYPE_RESERVED=0x08, // locked entry (not intel standard)
  TYPE_TSS=0x09, // state segment for a task
  TYPE_BUSYTSS=0x0B, // busy task state segment
  TYPE_TASKMASK=0x1D,
  TYPE_CALLGATE=0x0C,
  TYPE_INTGATE=0x0E,
  TYPE_TRAPGATE=0x0F,
  TYPE_GATEMASK=0x14, // x is a gate iff x & GATEMASK == GATETEST
  TYPE_GATETEST=0x04,
  TYPE_CLASSMASK=0x18,
  TYPE_DPL0=0x00,
  TYPE_DPL1=0x20,
  TYPE_DPL2=0x40,
  TYPE_DPL3=0x60,
  TYPE_DPLMASK=0x60,
} mal_descType_t;

/** subtype for data segments */
typedef enum {
  DATA_EXPDOWN=0x04, // this segment expands downward
  DATA_EXPUP=0,      // normal expand-up segment
  DATA_RONLY=0,      // read only segment
  DATA_RW=0x02,      // segment authorizes writing
  DATA_ACCESSED=0x01,// accessed since last test.
} mal_gdtDataFlags_t;

/** subtype for code segments */
typedef enum {
  CODE_CONFORMING=0x04,// conforming code (adapt to the requester's DPL)
  CODE_READABLE=0x02,  // code can be read as datas
  CODE_EXECONLY=0,     // code can only be executed
  CODE_ACCESSED=0x01,
} mal_gdtCodeFlags_t;

typedef enum {
  FLAG_32BIT=0x04, // 32 bits segments
  FLAG_16BIT=0, // 16 bits segments
  FLAG_GRAN1B=0, // 1 byte granularity (sizes up to 1Mb)
  FLAG_GRAN4K=0x08, // 4K granularity (sizes up to 4Gb)
  FLAG_AVL=0x01,
} mal_gdtDescrFlags_t;

typedef u_int16_t mal_selector_t;

#endif
