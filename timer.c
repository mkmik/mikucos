#include <timer.h>
#include <i8253.h>
#include <pic.h>
#include <hal.h>

#include <libc.h>
#include <thread.h>
#include <malloc.h>

volatile u_int64_t timer_ticks=0;

static LIST_HEAD(delay_list);

CREATE_ISR_DISPATCHER(timer);

static void timer_delay_hook();

void timer_init() {
  set_isr(0x20, timer_isr);
  i8253_init();
  enable_irq(0);
}

void timer_cisr() {
  timer_ticks++;
  if(!(timer_ticks & 0x1F))
    print_time();
  pic_ack(0);

  timer_delay_hook();

  // could cause context switch
  // delay hooks are executed first
  thread_timer_hook();
}

void print_time() {
  irql_t saved_irql = hal_irql_set(IRQL_INTERRUPT);

  int row, col;
  row = getRow();
  col = getCol();
  setPos(0, 50);
  int attr = getAttr(); 
  setAttr(0x7);

  dprintf(&libc_video_ops, "timer: %d      \n", (int)timer_ticks*1000/HZ);

  setAttr(attr); 
  setPos(row, col);
  
  hal_irql_restore(saved_irql);
}

void timer_spin_delay(int msec) {
  timetick_t time = timer_ticks;
  while(timer_ticks - time <= msec*HZ/1000);
}

// TODO: implement with mutex or condvar in order to be independent
// on thread runqueue api
static void timer_awake_thread(void *_thread) {
  tcb_t *thread = _thread;
  thread_wake(thread);
}

void timer_delay(int msec) {
  timer_register_delay(msec, timer_awake_thread, thread_current(), TIMER_ONESHOT);
  thread_sleep(thread_current());
}

timer_delay_entry_t* timer_register_delay(int msec, timer_callback_t callback, 
					  void *arg, int flags) {
  struct timer_delay_entry* entry = hmalloc(sizeof(struct timer_delay_entry));
  entry->delay_start = msec*HZ/1000;
  entry->delay = entry->delay_start;
  entry->callback = callback;
  entry->arg = arg;
  entry->flags = flags;

  irql_t saved_irql = hal_irql_set(IRQL_INTERRUPT);
  list_add_tail(&entry->delay_list, &delay_list);
  hal_irql_restore(saved_irql);

  return entry;
}

void timer_unregister_delay(timer_delay_entry_t* entry) {
  uprintf("unregistering delay %p\n");

  irql_t saved_irql = hal_irql_set(IRQL_INTERRUPT);
  list_del(&entry->delay_list);
  hal_irql_restore(saved_irql);

  // should free()
}

static void timer_delay_hook() {
  list_head_t *pos, *helper;
  timer_delay_entry_t* entry;

  // no need for locking since executing at interrupt irql

  list_for_each_safe(pos, helper,  &delay_list) {
    entry = list_entry(pos, struct timer_delay_entry, delay_list);

    if(--entry->delay <= 0) {
      timer_delay_entry_t en = *entry;

      if(en.flags & TIMER_ONESHOT)
	timer_unregister_delay(entry);

      en.callback(en.arg);

      if(!(en.flags & TIMER_ONESHOT))
	entry->delay = entry->delay_start;
    }
  }
}
