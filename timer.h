#ifndef __TIMER_H__
#define __TIMER_H__

#include <types.h>
#include <list.h>

#define HZ 500

typedef u_int64_t timetick_t;

extern volatile timetick_t timer_ticks;

/** initializes the timer controller and enables the timer irq */
void timer_init();
/** print the current time in milliseconds in to upper right corner */
void print_time();

/** delay execution of the current thread for "msec" milliseconds
 * this implementation blocks */
void timer_delay(int msec);
#define delay(msec) timer_delay(msec) // compat

/** delay execution of the current thread for "msec" milliseconds
 * this implementation spins */
void timer_spin_delay(int msec);

struct timer_delay_entry;
typedef struct timer_delay_entry timer_delay_entry_t;
typedef void (*timer_callback_t)(void* arg);

#define TIMER_ONESHOT 1

/** a subsystem cann register a callback function to be executed every "msec" 
 * milliseconds. The function will be executed at INTERRUPT irql.
 *  This can be used to simulate async IO and/or wake a thread which went to sleep.
 * if flags contain TIMER_ONESHOT, the callback is unregistered when fired. */
timer_delay_entry_t* timer_register_delay(int msec, timer_callback_t, 
					  void* arg, int flags);

void timer_unregister_delay(timer_delay_entry_t* entry);
// private

struct timer_delay_entry {
  struct list_head delay_list;
  timetick_t delay;
  timetick_t delay_start;
  timer_callback_t callback;
  void* arg;
  int flags;
};

#endif
