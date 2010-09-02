#ifndef __PROCESS__
#define __PROCESS__

#include <thread.h>
#include <file.h>

#define MAX_FD 256

struct pcb {
  /** file descriptor table */
  file_t* fd[MAX_FD];
  /** every process has its root directory */
  fnode_t *root;
  /** current working directory.
   * A string imposes less constrains on what
   * the underlining system. For example what if 
   * the cwd is deleted while a process is in it:
   * we don't keep track of it */
  char *cwd;
};
typedef struct pcb pcb_t;

/** module initialization function */
extern void process_init();

/** returns the current process 
 * (the process associated to the current thread) */
extern struct pcb* process_current();

/** private. initialize a new pcb structure */
extern void process_init_pcb(struct pcb *pcb);

#endif
