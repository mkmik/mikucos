#include <shell_builtins.h>

#include <file_api.h>
#include <version.h>
#include <hal.h>
#include <process.h>

#include <ide.h>

#include <user_programs/internals.h>

#define BUILTIN(nm)  { .name = #nm, .builtin = builtin_ ## nm , \
                       .flags =  SHELL_APP_BUILTIN, .by_name = null_avl_node }
#define INTERNAL(nm) { .name = #nm, .internal= internal_ ## nm , \
                       .flags =  SHELL_APP_INTERNAL, .by_name = null_avl_node}

static shell_application_t builtins[] = {
  BUILTIN(builtins),
  BUILTIN(shell), // the shell itself
  BUILTIN(cd),
  BUILTIN(pwd),
  BUILTIN(echo),
  BUILTIN(exit),
  INTERNAL(ls),
  INTERNAL(cat),
  INTERNAL(uname),
  INTERNAL(reboot),
  INTERNAL(ide_test),
  INTERNAL(fdisk),
  INTERNAL(vesa),
  INTERNAL(mount),
  INTERNAL(ip),
  INTERNAL(net_test),
//  INTERNAL(flood),
};

void shell_builtins_fill(shell_t *shell) {
  int i;
  for(i=0; i < sizeof(builtins)/sizeof(*builtins); i++) {
    shell_application_t *copy = malloc(sizeof(shell_application_t));
    memcpy(copy, &builtins[i], sizeof(shell_application_t));
    shell_add_application(shell, copy);
  }
}

int builtin_builtins(shell_t* this, int argc, char** argv) {
  list_head_t *pos;
  list_for_each(pos, &this->applications) {
    shell_application_t *app = list_entry(pos, shell_application_t, list);
    write(stdout_fd, app->name, strlen(app->name));
    write(stdout_fd, " ", 1);
  }
  write(stdout_fd, "\n", 1);
  return 0;
}



// THIS IS KAOS!!!! memory leaks everywhere
int builtin_cd(shell_t* this, int argc, char** argv) {
  char *nwd;
  char *oldcwd = process_current()->cwd;

  if(argc > 1) 
    nwd = argv[1];
  else {
    process_current()->cwd = strdup("/");
    return 0;
  }

  // go to parent
  if(strcmp(nwd, "..") == 0) {
    if(strcmp(oldcwd, "/") != 0) {
      nwd = file_path_dirname(oldcwd);
      if(strlen(nwd) == 0)
	nwd = strdup("/");
    } else 
      return 0;
  }
  
  char* newcwd = 0;
  // relative paths
  if(*nwd != '/') {
    newcwd =  malloc(strlen(nwd) + strlen(oldcwd) + 1);
    strcpy(newcwd, oldcwd);
    if(strcmp(oldcwd, "/") != 0) // exception to prevent //path, bug in resolver
      strcat(newcwd, "/");
    strcat(newcwd, nwd);
    
  } else {
    newcwd = strdup(nwd);
  }

  fnode_t *fnode = file_resolve_path(newcwd);
  if(fnode == 0) {
    xprintf("no such directory '%s'\n", newcwd);
    return 1;
  }
  if(!(fnode->flags & DIRENT_DIR)) {
    xprintf("'%s' is not a directory\n", newcwd);
    return 1;
  }
  // TODO: memory leak. Who allocates fnodes, who caches and who frees them ?
  //  free(fnode);

  process_current()->cwd = newcwd;

  return 0;
}

int builtin_pwd(shell_t* this, int argc, char** argv) {
  char *cwd = process_current()->cwd;
  write(stdout_fd, cwd, strlen(cwd));
  write(stdout_fd, "\n", 1);
  return 0;
}

int builtin_echo(shell_t* this, int argc, char** argv) {
  int i;
  for(i=1; i<argc; i++) {
    if(i > 1)
      printf(" ");
    printf("%s", argv[i]);
  }
  printf("\n");
  return 0;
}

int builtin_shell(shell_t* this, int argc, char** argv) {
  shell_run_args(argc, argv);
  return 0;
}

int builtin_exit(shell_t* this, int argc, char** argv) {
  this->ready_to_exit = 1;
  return 0;
}

int internal_reboot(int argc, char** argv) {
  hal_reboot();
  return 127;
}
