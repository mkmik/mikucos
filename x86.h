#ifndef __X86_H__
#define __X86_H__

#include <types.h>
#include <config.h>

/** interrupt descriptor.
 * contains the segment and offset
 * of the interrupt handler routine
 * and flags */
struct interrupt_desc {
  u_int16_t lowoff;
  u_int16_t segment;
  u_int16_t flags;
  u_int16_t highoff;
};

/** I give up structuring this one :-(
 * it seems that intel developers had some
 * mental desease ... */
struct segment_desc {
  u_int64_t value;
};

/** Page Table Entry.
 * Page tables (and the page directory) are arrays of 
 * pte_t elements. */
typedef struct {
  u_int32_t data;
} pte_t;

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

/** this function creates a TSS descriptor using "addr" as TSS struct address */
void set_tss_desc(struct segment_desc* desc, void *addr);

/** "paging enable" bit */
#define CR0_PG  0x80000000
/** "page size extension enable" bit */
#define CR4_PSE (1L<<4)
/** "physical address extension enable" bit */
#define CR4_PAE (1L<<5)

#define WRITE_CR0(value) asm("mov %0, %%cr0" : :"r"(value))
#define READ_CR0(value) asm("mov %%cr0, %0" :"=r"(value))
#define WRITE_CR3(value) asm("mov %0, %%cr3" : :"r"(value))
#define READ_CR3(value) asm("mov %%cr3, %0"  :"=r"(value))
#define WRITE_CR4(value) asm("mov %0, %%cr4" : :"r"(value))
#define READ_CR4(value) asm("mov %%cr4, %0"  :"=r"(value))
#define WRITE_PAGE_DIRECTORY(value) WRITE_CR3(value)
#define READ_PAGE_DIRECTORY(value) READ_CR3(value)

/** enable paging (flushing instruction queue) */
#define ENABLE_PAGING() do { \
 u_int32_t tmp; \
 READ_CR0(tmp); \
 WRITE_CR0(tmp | CR0_PG); \
 asm("jmp 1f\n1:\n\tmovl $1f,%0" : "=r"(tmp)); \
 asm("jmp *%0\n1:" : : "r"(tmp)); \
} while(0)

#define ENABLE_PSE() do { \
 u_int32_t tmp; \
 READ_CR4(tmp); \
 WRITE_CR4(tmp | CR4_PSE); \
} while(0)

/** flush only the tlb entry for the page which contains the specified address */
#define flush_tlb_one(addr) asm volatile("invlpg %0": :"m" (*(char *) addr))

/* code from linux msr.h: */

/* 
 * Access to machine-specific registers (available on 586 and better only)
 * Note: the rd* operations modify the parameters directly (without using
 * pointer indirection), this allows gcc to optimize better
 */

#define rdmsr(msr,val1,val2) \
     __asm__ __volatile__("rdmsr" \
                          : "=a" (val1), "=d" (val2) \
                          : "c" (msr))
#define wrmsr(msr,val1,val2) \
     __asm__ __volatile__("wrmsr" \
                          : /* no outputs */ \
                          : "c" (msr), "a" (val1), "d" (val2))

#define MSR_IA32_APICBASE               0x1b
#define MSR_IA32_APICBASE_BSP           (1<<8)
#define MSR_IA32_APICBASE_ENABLE        (1<<11)
#define MSR_IA32_APICBASE_BASE          (0xfffff<<12)
#define MSR_IA32_SYSENTER_CS            0x174
#define MSR_IA32_SYSENTER_ESP           0x175
#define MSR_IA32_SYSENTER_EIP           0x176


#if CONFIG_MPENTIUMIII
#define ARCH_HAS_PREFETCH
extern inline void prefetch(const void *x)
{
        __asm__ __volatile__ ("prefetchnta (%0)" : : "r"(x));
}
#else
static inline void prefetch(const void *x) {;}
#endif

/* end linux */

#endif
