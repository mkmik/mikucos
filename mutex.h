#ifndef _MUTEX_H_
#define _MUTEX_H_

#include <thread.h>
#include <spin_lock.h>

struct mutex {
  /** this is the real lock */
  spin_lock_t spin_lock;
  /** stores threads that wait on the mutext */
  list_head_t lazy_list;
};
typedef struct mutex mutex_t;

extern void mutex_init_mutex(mutex_t *mutex);
extern int mutex_lock(mutex_t *mutex);
extern int mutex_trylock(mutex_t *mutex);
extern int mutex_unlock(mutex_t *mutex);
#endif
