#ifndef _CONDVAR_H_
#define _CONDVAR_H_

#include <mutex.h>

struct condvar {
  list_head_t lazy_list;
};
typedef struct condvar condvar_t;

extern void condvar_init_condvar(condvar_t *condvar);
extern int condvar_signal(condvar_t *condvar);
extern int condvar_broadcast(condvar_t *condvar);
extern int condvar_wait(condvar_t *condvar, mutex_t *mutex);
extern int condvar_destroy(condvar_t *condvar);

#endif
