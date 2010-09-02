#ifndef _PLATFORM_MAL_GDT_H_
#define _PLATFORM_MAL_GDT_H_

// from acaros

#include <desc.h>

/** Task State Segment structure.
 * This is the structure of memory pointed by
 * the Task State Segment descriptor.
 * The system has only one global TSS struct
 * because it uses sofware context switches (faster and flexible).
 * The TSS is still needed because it contains kernel mode stack (ring 0 stack)
 * in esp0, which is switched automatically by the CPU when the execution passes
 * from user-mode to kernel-mode (ring3->ring0) because, for example, of an interrupt.
 *  Only the esp0 and ss0 fields are used
 */
struct tss_struct {
  u_int16_t  back_link,__blh;
  u_int32_t   esp0;
  u_int16_t  ss0,__ss0h;
  u_int32_t   esp1;
  u_int16_t  ss1,__ss1h;
  u_int32_t   esp2;
  u_int16_t  ss2,__ss2h;
  u_int32_t  __cr3;
  u_int32_t  eip;
  u_int32_t  eflags;
  u_int32_t  eax,ecx,edx,ebx;
  u_int32_t  esp;
  u_int32_t  ebp;
  u_int32_t  esi;
  u_int32_t  edi;
  u_int16_t  es, __esh;
  u_int16_t  cs, __csh;
  u_int16_t  ss, __ssh;
  u_int16_t  ds, __dsh;
  u_int16_t  fs, __fsh;
  u_int16_t  gs, __gsh;
  u_int16_t  ldt, __ldth;
  u_int16_t  trace, bitmap;
  u_int32_t   io_bitmap[33];
};

void mal_gdtInit();

mal_segment_t* mal_descriptorAtSelector(mal_selector_t sel);

#endif
