#include <shell.h>
#include <shell_builtins.h>
#include <malloc.h>

#include <file_api.h>
#include <virtual_console.h>
#include <keyboard.h>
#include <keyboard_api.h>
#include <termios.h>
#include <version.h>
#include <process.h>

void shell_init_shell(shell_t *shell) {
  shell->applications_by_name = 0;
  shell->interactive = 1;
  shell->ready_to_exit = 0;
  shell->input = stdin;
  shell->prompt = "\e[01;32mroot@lokaost \e[01;34m%s # \e[00m";

  INIT_LIST_HEAD(&shell->applications);
  shell_builtins_fill(shell);
}

void shell_init_application(shell_application_t *app) {
  app->name = "";
  app->builtin = 0;
  app->internal = 0;
  app->path = 0;
  app->flags = SHELL_APP_BUILTIN;
  app->by_name = null_avl_node;
}

void shell_add_application(shell_t *this, shell_application_t *app) {
  avl_add(&app->by_name, &this->applications_by_name, shell_application_t);
  list_add_tail(&app->list, &this->applications);
}

shell_application_t* shell_find_application(shell_t* this, const char* name) {
  return avl_find(name, this->applications_by_name,
		  shell_application_t, by_name);
}

int shell_execute_application(shell_t *this, shell_application_t* app,
			      int argc, char** argv) {
  if(app->flags == SHELL_APP_BUILTIN) {
    assert(app->builtin);
    return app->builtin(this, argc, argv);
  } else if(app->flags == SHELL_APP_INTERNAL) {
    assert(app->internal);
    return app->internal(argc, argv);
  } else if(app->flags == SHELL_APP_EXTERNAL)
    panicf("execution of external programs is not yet implemented\n");
  else 
    panicf("flag not defined");
}

void shell_run_args(int argc, char** argv) {
  shell_t *shell = shell_create();

  int i;
  int none = 1;
  for(i=1; i<argc; i++) {
    none = 0;
    if(!(strcmp(argv[i], "-c"))) {
      // not implemented
    } else {
      shell->input = fopen(argv[i], "r");
      if(shell->input == 0) {
	printf("cannot open '%s'\n", argv[i]);
	return;
      }
    }
  }

  shell_main(shell);

  shell_destroy(shell);
}

void shell_run() {
  char *argv[] = {"shell", "/System/Startup/systartup.com"};
  int argc = 1;
  
  int in, out;
  out = open("/Devices/VirtualConsole/0", 0);
  if(out == -1)
    panicf("cannot open /Devices/VirtualConsole/0");
  in = out;

  // hard code file descriptors in stdin_fd, stdout_fd, and stderr_fd
  dup2(in, stdin_fd);
  dup2(out, stdout_fd);
  dup2(out, stderr_fd);

  use_vfs_console(stdin_fd, stdout_fd); // switch to vfs based printf and scanf
  // erase screen
  printf("\e[2J");

  // if startup file exists then execute it
  in = open(argv[1], 0);
  if(in != -1) {
    argc++;
    close(in);
  }

  shell_run_args(argc, argv);
}

shell_t* shell_create() {
  shell_t* shell = malloc(sizeof(shell_t));
  shell_init_shell(shell);
  return shell;
}

void shell_destroy(shell_t* this) {
  // destruct builtins
  list_head_t *pos, *tmp;
  list_for_each_safe(pos, tmp, &this->applications) {
    free(list_entry(pos, shell_application_t, list));
  }

  free(this);
}

static void shell_print_prompt(shell_t *this) {
  if(this->interactive) {
    char *last = file_path_filename(process_current()->cwd);
    printf(this->prompt, last ? : "/");
    if(last)
      free(last);
  }
}

#define MAX_ARGC 20

void shell_main(shell_t *this) {
  char buffer[256];
  char *argv[MAX_ARGC];

  termios_t termios;
  int res = get_device_option(fileno(this->input), "termios", &termios);
  if(res == -1)
    this->interactive = 0;

  while(!this->ready_to_exit) {
    char *cmdline;
    int argc = 0;

    shell_print_prompt(this);

    cmdline = fgets(buffer, sizeof(buffer), this->input);
    if(!cmdline) {
      if(ferror(stdin))
	printf("error reading stdin\n");
      break;
    }

    while(argc < MAX_ARGC) {
      char old;
      if(!(*cmdline && *cmdline != '\n'))
	 break;

      argv[argc++] = cmdline;
      while(*cmdline && *cmdline != ' ' && *cmdline != '\n')
	cmdline++;
      old = *cmdline;
      *cmdline = 0;
      if(old == ' ') {
	do { 
	  cmdline++; 
	} while(*cmdline == ' ');
      } else
	break;
    }

    if(argc == 0)
      continue;

    shell_application_t *app = shell_find_application(this, argv[0]);
    if(app)
      shell_execute_application(this, app, argc, argv);
    else
      printf("%s: command not found\n", argv[0]);
  }
}
