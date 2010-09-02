#ifndef __APPLICATION_H__
#define __APPLICATION_H__

/** 
 * Applications are identified with a text string (name)
 * which is matched to the kernel command line. 
 * If the cmdline contains the name of the application then 
 * it is "active" and is run calling "init" and "run" methods
 * through applications_init() and applications_run().
 *  If "thread" is true the application will run in it's own
 * thread, otherwise the caller will wait that the application
 * returns.
 */
struct application_t {
  char* name;
  void (*init)();
  void (*run)();
  int thread; /* run inside a separate thread */
  int stack_size;
};

/** application array (application.c) */
extern struct application_t applications[];

/** iterates over all active applications and run their "init" method */
void applications_init();
/** iterates over all active applications and run their "run" method */
void applications_run();

#endif

