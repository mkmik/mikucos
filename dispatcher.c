#include <dispatcher.h>

/** only the scheduler knows the last context.
 * the dispatcher is in the microkernel and doesn't
 * want to know about policy. This little hack is needed 
 * to properly switch. */
static struct dispatcher_context *current_context = 0;

/** this functions switch to the given the context. 
 * This function doesn't want to know anything about
 * policy and scheduling algorithm, it just do 
 * thread dispatching (as called in VMS & WNT).
 *  Ideally this would be in the microkernel while
 * the scheduler in a subsystem (perhaps usermode) */
void dispatcher_switch_to(struct dispatcher_context* next) {
  struct dispatcher_context *prev = current_context;
  current_context = next;
  asm("pusha");
  __dispatcher_switch_to(prev, next, prev);
  
  asm("popa");
}

extern void dispatcher_init(struct dispatcher_context *initial) {
  current_context = initial;
}
