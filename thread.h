#ifndef __THREAD_H__
#define __THREAD_H__

/** TODO: error handling */

#include <dispatcher.h>
#include <list.h>
#include <timer.h>
#include <malloc.h>
#include <avl.h>
#include <spin_lock.h>

#define THREAD_INITIAL_QUANTUM (10*HZ/1000) // 10 msec

/** Thread Control Block */
struct tcb {
  struct dispatcher_context context;
  struct list_head run_list;
  /** time quantum assigned to the thead.
   * decremented at each timer tick.
   * when it reaches 0 the thread is preempted */
  volatile int quantum; // must be signed for debug checks to work
  /** string map of per thread private data */
  avl_node_t *thread_private;
};
typedef struct tcb tcb_t;

struct thread_private {
  char* key;
  void* data;
  avl_node_t node;
};
typedef struct thread_private thread_private_t;
avl_make_string_compare(thread_private_t, node, key);
avl_make_string_match(thread_private_t, key);

/** module initialization function */
extern void thread_init();

/** schedule next ready thread */
extern void thread_schedule();

/** schedule next ready thread
 * but put the old in the list specified by the argument */
extern void thread_schedule_preempted_in_list(list_head_t *list);

/** returns the current thread */
extern struct tcb* thread_current();

#define THREAD_DEFAULT_STACK_SIZE 0x400

/** this structure is used to pass optional parameters to 
 * the thread_create function */
struct thread_create_attrs {
  /** allocate if zero */
  void *stack;
  /** defaults to THREAD_DEFAULT_STACK_SIZE if zero */
  unsigned  stack_size;
};

/** create a thread.
 * "func" is the entry point, "arg" is a pointer to a buffer containg
 * arguments, and "attrs", if not null,  points to optional parameters */
extern struct tcb* thread_create(void (*func)(void*), 
				 void *arg,
				 struct thread_create_attrs* attrs);

/** put a thread into sleeping state.
 * this involves removing it from the runqueue
 */
extern void thread_sleep(tcb_t *thread);

/** put a thread into the runqueue and
 * (TODO) if higher priority than caller then reschedule 
 */
extern void thread_wake(tcb_t *thread);

/** called when a timer tick occours */
extern void thread_timer_hook();

/** get private data from string */
extern void* thread_get_private_data(struct tcb* tcb, char* key);

/** associate a private data to a string */
extern void thread_set_private_data(struct tcb* tcb, char* key, void* data);

/** the first and the last thread in the system. 
 * this thread is called when nothing else runs.
 * Currently is called as other threads because 
 * priorities are not implemented */
extern struct tcb* idle_tcb;

// TODO: opaquize this!
extern list_head_t runqueue_head;
extern tcb_t* current_tcb;

#endif
