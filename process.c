#include <process.h>
#include <malloc.h>
#include <libc.h>

struct pcb *kernel_process;

void process_init() {
  kernel_process = malloc(sizeof(struct pcb));
  process_init_pcb(kernel_process);
}

void process_init_pcb(struct pcb *pcb) {
  int i;
  for(i=0; i<MAX_FD; i++)
    pcb->fd[i] = 0;
  pcb->root = 0;
  pcb->cwd = "/";
}

struct pcb* process_current() {
  struct tcb* tcb = thread_current();
  struct pcb* pcb = (struct pcb*)thread_get_private_data(tcb, "_S_PCB");

  if(!pcb) {
    pcb = kernel_process;
    thread_set_private_data(tcb, "_S_PCB", pcb);
  }
  return pcb;
}
