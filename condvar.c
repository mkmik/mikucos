#include <condvar.h>
#include <hal.h>
#include <thread.h>
#include <libc.h>
#include <debug.h>

void condvar_init_condvar(condvar_t *condvar) {
  INIT_LIST_HEAD(&condvar->lazy_list);
}

int condvar_signal(condvar_t *condvar) {
  irql_t saved_irql = hal_irql_set(IRQL_DISPATCH);


  if(!list_empty(&condvar->lazy_list)) {
    list_head_t *tmp = condvar->lazy_list.next;
    list_del(tmp);
    list_add_tail(tmp, &runqueue_head);
  }

  hal_irql_restore(saved_irql);

  return 0;
}

int condvar_broadcast(condvar_t *condvar) {
  irql_t saved_irql = hal_irql_set(IRQL_DISPATCH);

  list_head_t *pos,*tmp;
  list_for_each_safe(pos, tmp, &condvar->lazy_list) {
    list_del(pos);
    list_add_tail(pos, &runqueue_head);
  }

  hal_irql_restore(saved_irql);

  return 0;
}

int condvar_wait(condvar_t *condvar, mutex_t *mutex) {
  irql_t saved_irql = hal_irql_set(IRQL_DISPATCH);

  mutex_unlock(mutex);

  // ~ cut&paste from thread.c
  struct tcb *prev = list_entry(runqueue_head.next, struct tcb, run_list);
  struct tcb *next;
  
  list_del(&prev->run_list);
  list_add_tail(&prev->run_list, &condvar->lazy_list);
  next = list_entry(runqueue_head.next, struct tcb, run_list);
  current_tcb = next;
  
  if(next->quantum <= 0)
    next->quantum = THREAD_INITIAL_QUANTUM; 

  hal_irql_restore(saved_irql);

  dispatcher_switch_to(&next->context);
  // end ~ cut&paste

  mutex_lock(mutex);
  
  return 0;
}

int condvar_destroy(condvar_t *condvar) {
  assert(!"not implemented yet");

  return 0;
}


