#include "thread_test.h"
#include <thread.h>
#include <libc.h>
#include <timer.h>
#include <hal.h>
#include <config.h>
#include <mutex.h>

#define THREADS 4
struct thread_runner_args {
  int row;
  int col;
  char *string;
  int attr[2];
  int freq;
  int state;
} runner_args[THREADS] ={
  {5, 10, "ciao", {0x7, 0x17},4, 0},
  {15, 10, "miao", {0x5, 0x13},3, 0},
  {5, 20, "haha", {0x11, 0x14},5, 0},
  {15, 20, "hoho", {0x6, 0x4},8, 0}
};

void thread_runner(void* _arg) {
  struct thread_runner_args *arg = (struct thread_runner_args*)_arg;
  int row, col, attr;
  
  while(1) {
    irql_t saved_irql = hal_irql_set(IRQL_DISPATCH);
   
    row = getRow();
    col = getCol();
    attr = getAttr();
    
    setPos(arg->row, arg->col);
    setAttr(arg->attr[arg->state++ % 2]);
    printf("%s", arg->string);
    
    setAttr(attr);
    setPos(row, col);
    
    hal_irql_restore(saved_irql);

    delay(1000/arg->freq);
  }
}

mutex_t pc_mutex; // producer-consumer mutex

static void producer(void * arg) {
  xprintf("produced starting\n");

  while(1);

  while(1) {
    mutex_lock(&pc_mutex);
    xprintf("Producer: lock acquired \n");
    delay(4000);
    xprintf("Producer: produced releasing lock\n");
    mutex_unlock(&pc_mutex);
  }
}

static void consumer() {
  delay(1000);

  while(1);

  while(1) {
    xprintf("Consumer: trying to acquire lock\n");
    mutex_lock(&pc_mutex);
    xprintf("Consumer: lock acquired\n");
    xprintf("Consumer: doing ");
    delay(500);
    xprintf("... done\n");
    mutex_unlock(&pc_mutex);
    xprintf("Consumer: lock released\n");
    delay(2000);
  }
}

void thread_test_init() {
  mutex_init_mutex(&pc_mutex);
}

void thread_test_run() {
  int i;
  
  printf("initializing ... ");
  for(i=0; i<THREADS;i++)
    thread_create(thread_runner, &runner_args[i], 0);

  //  thread_create(producer, 0, 0);
  //  thread_create(consumer, 0, 0);
  printf("done\n");  

  while(1);
}
