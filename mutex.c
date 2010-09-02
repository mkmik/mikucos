#include <mutex.h>
#include <debug.h>
#include <libc.h>
#include <hal.h>

void mutex_init_mutex(mutex_t *mutex) {
  spin_lock_init(&mutex->spin_lock);
  INIT_LIST_HEAD(&mutex->lazy_list);
}

int mutex_lock(mutex_t *mutex) {
  while(1) {
    if(spin_try_lock(&mutex->spin_lock))
      return 0;
    
    thread_schedule_preempted_in_list(&mutex->lazy_list);    
  }

  return 0;
}

int mutex_trylock(mutex_t *mutex) {
  return spin_try_lock(&mutex->spin_lock);
}

int mutex_unlock(mutex_t *mutex) {
  assert(spin_lock_locked(&mutex->spin_lock));

  irql_t saved_irql = hal_irql_set(IRQL_DISPATCH);

  list_head_t *pos,*tmp;
  list_for_each_safe(pos, tmp, &mutex->lazy_list) {
    list_del(pos);
    list_add_tail(pos, &runqueue_head);
  }

  spin_unlock(&mutex->spin_lock);

  hal_irql_restore(saved_irql);

  return 0;
}

