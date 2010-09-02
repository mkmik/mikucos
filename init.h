#ifndef __INIT_H__
#define __INIT_H__

#define INIT_STACK_SIZE (4096*8)

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

// Code below cannot be included from a .S file
// TODO: cleanup the entry.S code and initialize segments from C.

#ifndef __ASSEMBLY__

#include <hal.h>
#include <multiboot.h>
#include <x86.h>

/** Initializes the kernel, beginning with simple 
 * IO/debug routines (vga, uart) and ending with the threading 
 * envirnonment. Interrupts are enabled and the main thread
 * (running main() in main.c) is started. */
extern void init();

/** Initial stack used by the kernel during booting.
 * Once threading is started every kernel thread has a separate stack
 * which is allocated from the kernel heap (malloc) */
extern char init_stack[];

/** This is the "interruptor descriptor table",
 * a i386 specific structure that describes interrupt entry points.
 *  Probably this should be put in a platform specific module */
extern struct interrupt_desc idt_table[];
/** This is the "global descriptor table" which is a
 * i386 specific structure that describes system-wide segments.
 *  See above for portability remarks */
extern struct segment_desc gdt_table[];

#endif

#endif
