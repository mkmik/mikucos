#include <thread.h>
#include <libc.h>
#include <timer.h>
#include <hal.h>
#include <list.h>
#include <malloc.h>
#include <spin_lock.h>
#include <uart.h>

LIST_HEAD(runqueue_head);

struct tcb *current_tcb = 0;

/** The scheduler chooses the next thread to run,
 * and then calls dispatcher_switch_to.
 *  The scheduler implements a stupid "one priority" fifo
 * alogorthm. in other words: a thread executes until it chooses
 * to yield (*). Preemption occours when the timer interrupt
 * yields the current thread when it has consumed its quantum.
 * It will be changed to a priority queue scheduler.
 * (*: interrupts are, from the scheduler point of view, executed in
 * the context of the current thread)
 */
void thread_schedule() {
  thread_schedule_preempted_in_list(&runqueue_head);
}

void thread_schedule_preempted_in_list(list_head_t *preempted_list) {
  irql_t saved_irql = hal_irql_set(IRQL_DISPATCH);

  struct tcb *prev = list_entry(runqueue_head.next, struct tcb, run_list);
  struct tcb *next;
  
  list_del(&prev->run_list);
  list_add_tail(&prev->run_list, preempted_list);
  next = list_entry(runqueue_head.next, struct tcb, run_list);
  current_tcb = next;
  
  if(next->quantum <= 0)
    next->quantum = THREAD_INITIAL_QUANTUM; 

  hal_irql_restore(saved_irql);

  dispatcher_switch_to(&next->context);

}

static volatile int cpu_load = 0;
static void print_cpu_load();

void thread_timer_hook() {
  static int cpu_load_printing = 0;

  if(!(cpu_load_printing++ % 125)) {
    print_cpu_load();
    cpu_load = 0;
  }

  if(current_tcb) {
    if((--current_tcb->quantum) > 0)
      return; 
   
    hal_enable_interrupts(); // TODO: find a better place
    thread_schedule();
  }
}

struct tcb* idle_tcb = 0;
	
static void idle_thread() {
  u_int32_t i;
  thread_schedule(); // first context switch
  hal_enable_interrupts();
  while(1) {
    for(i=0; i<0xFF; i++);
    cpu_load++;
    if(cpu_load == 0xFFFFFFFF)
      cpu_load = 0xFFFFFFFF-1;
  }
}

void thread_init() {
  idle_tcb = thread_create(idle_thread, 0, 0);
  dispatcher_init(&idle_tcb->context);
  current_tcb = idle_tcb; // necessary only to use thread_current before scheduler starts
}

static void thread_returned() {
  struct tcb * prev;

  irql_t saved_irql = hal_irql_set(IRQL_DISPATCH);

  prev = list_entry(runqueue_head.next, struct tcb, run_list);  
  list_del(&prev->run_list);

  hal_irql_restore(saved_irql);

  thread_schedule();
}

struct tcb* thread_create(void (*func)(void*), 
			  void *arg,
			  struct thread_create_attrs* attrs) {
  struct thread_create_attrs default_attrs = {0,};
  struct tcb* tcb = malloc(sizeof(struct tcb));
  
  if(attrs == 0) 
    attrs = &default_attrs;
  if(attrs->stack_size == 0)
    attrs->stack_size = THREAD_DEFAULT_STACK_SIZE;
  if(attrs->stack == 0)
    attrs->stack = malloc(attrs->stack_size);
  
  tcb->context.eip = (u_int32_t)func;
  tcb->context.esp = (u_int32_t)attrs->stack + (u_int32_t)attrs->stack_size - 0x30; 
  // 0x30 margin because __dispatcher_switch_to() pops all registers from the stack
  
  *--((u_int32_t*)tcb->context.esp) = (u_int32_t)arg; // init stacks
  *--((u_int32_t*)tcb->context.esp) = (u_int32_t)thread_returned; // thread returns here

  tcb->quantum = THREAD_INITIAL_QUANTUM;
  tcb->thread_private = 0;

  thread_wake(tcb);
  
  return tcb;
}

extern void thread_sleep(tcb_t *thread) {
  irql_t saved_irql = hal_irql_set(IRQL_DISPATCH);

  list_del(&thread->run_list);

  hal_irql_restore(saved_irql);

  if(thread == thread_current())
    thread_schedule();
}

extern void thread_wake(tcb_t *thread) {
  irql_t saved_irql = hal_irql_set(IRQL_DISPATCH);

  list_add_tail(&thread->run_list, &runqueue_head);

  hal_irql_restore(saved_irql);
}

struct tcb* thread_current() {
  return current_tcb;
}

extern void* thread_get_private_data(struct tcb* tcb, char* key) {
  thread_private_t *res;
  res = avl_find(key, tcb->thread_private, thread_private_t, node);
  if(!res)
    return 0;
  return res->data;
}

extern void thread_set_private_data(struct tcb* tcb, char* key, void* data) {
  thread_private_t* priv = malloc(sizeof(thread_private_t));
  priv->node = null_avl_node;
  priv->data = data;
  priv->key = key;
  avl_add(&priv->node, &tcb->thread_private, thread_private_t);
}

static void print_cpu_load() {
 irql_t saved_irql = hal_irql_set(IRQL_DISPATCH);

 int row, col;
 row = getRow();
 col = getCol();
 setPos(1, 50);
 int attr = getAttr(); 
 setAttr(0x7);
 
 dprintf(&libc_video_ops, "idle: %d      \n", (cpu_load/0x100*4));
 
 setAttr(attr); 
 setPos(row, col);

 hal_irql_restore(saved_irql);
}
