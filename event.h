#ifndef __EVENT_H__
#define __EVENT_H__

#include <list.h>

struct event {
  struct list_head wait_list;
  enum {ALL, FIRST} wakeup_mode;
};

/** puts current thread on sleep 
 * until the event is sigaled */
extern void event_wait_on(struct event* evt);

/** signals the event waking up the threads
 * in the wait list according to "mode" */
extern void event_signal(struct event* evt);

#endif
