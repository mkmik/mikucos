#ifndef _SHELL_H_
#define _SHELL_H_

#include <types.h>
#include <avl.h>
#include <list.h>
#include <stdio.h>

struct shell {
  avl_node_t *applications_by_name;
  list_head_t applications;
  int ready_to_exit;
  int interactive;
  FILE* input;
  char* prompt;
};
typedef struct shell shell_t;

/** special application called in the context of the shell */
typedef int (*shell_application_builtin_t)(shell_t* this, int argc, char **argv);
/** application that is resident in the kernel but not necessarly is aware of it */
typedef int (*shell_application_main_t)(int argc, char **argv);

#define SHELL_APP_BUILTIN 1
#define SHELL_APP_INTERNAL 2
#define SHELL_APP_EXTERNAL 3

struct shell_application {
  /** name of the application */
  const char* name;
  /** pointer to the builtin routine */
  shell_application_builtin_t builtin;
  /** pointer to the internal routine */
  shell_application_main_t internal;
  /** path of the external program */
  const char* path;

  /** builtin or external */
  int flags; 

  /** tree sorted by name */
  avl_node_t by_name;
  /** all applications. for enumeration */
  list_head_t list;
};
typedef struct shell_application shell_application_t;
avl_make_string_compare(shell_application_t, by_name, name);
avl_make_string_match(shell_application_t, name);


/** initialize a shell adding default builtins */
void shell_init_shell(shell_t *shell);

void shell_init_application(shell_application_t *app); 

void shell_add_application(shell_t *this, shell_application_t *app);

/** returns the application matching 'name' or NULL if not found */
shell_application_t* shell_find_application(shell_t *this, const char* name);

int shell_execute_application(shell_t *this, shell_application_t* app,
			      int argc, char** argv);

/** run a shell on a newly allocated virtual console 
 * (TODO. now binds on the first one) */
void shell_run();

/** same as above but accepts command line args
 * useful for subshells */
void shell_run_args(int argc, char** argv);

/** allocates and initializes a shell */
shell_t* shell_create();

/** destroy shell object and dependants (builtins) */
void shell_destroy(shell_t* this);

/** shell main loop. it needs an already initialized shell */
void shell_main(shell_t *this);

#endif
