#ifndef __DISPATCHER_H__
#define __DISPATCHER_H__

#include <types.h>

/* saved machine context 
 * for a thread. The registers are saved 
 * saved on the stack just before calling
 * thread_switch_to. This structure contains
 * only the stack pointer and the instruction pointer. */
struct dispatcher_context {
  u_int32_t esp;
  u_int32_t eip;
};

extern void dispatcher_init(struct dispatcher_context *initial);

/** performs a context switch */
extern void dispatcher_switch_to(struct dispatcher_context *next);

// private

/** don't use directly. use thread_dispatch */

// this code implicitly saves all registers on the stack 
// and then switches the stacks and pops all back
#define __dispatcher_switch_to(prev,next,last) do { \
  asm volatile("pushl %%esi\n\t"                                  \
		 "pushl %%edi\n\t"                                  \
		 "pushl %%ebp\n\t"                                  \
		 "movl %%esp,%0\n\t"        /* save ESP */          \
		 "movl %3,%%esp\n\t"        /* restore ESP */       \
		 "movl $1f,%1\n\t"          /* save EIP */          \
		 "pushl %4\n\t"             /* restore EIP */       \
		 "ret\n"         /* or "call" hook for c code */      \
		 "1:\t"                                             \
		 "popl %%ebp\n\t"                                   \
		 "popl %%edi\n\t"                                   \
		 "popl %%esi\n\t"                                   \
		 :"=m" (prev->esp),"=m" (prev->eip),  \
		  "=b" (last)                                       \
		 :"m" (next->esp),"m" (next->eip),    \
		  "a" (prev), "d" (next),                           \
		  "b" (prev));                                      \
} while (0)



#endif
