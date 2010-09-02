#ifndef _SHELL_BUILTINS_H_
#define _SHELL_BUILTINS_H_

#include <shell.h>

/** fills builtins and internals */
void shell_builtins_fill(shell_t *shell);

/** prints out the names of the shell builtin commands 
 * currently registered in the shell */
int builtin_builtins(shell_t* this, int argc, char** argv);

/** spawn a new shell instance */
int builtin_shell(shell_t* this, int argc, char** argv);

/** changes current working directory */
int builtin_cd(shell_t* this, int argc, char** argv);

/** prints current working directory */
int builtin_pwd(shell_t* this, int argc, char** argv);

/** echoes arguments to stdout_fd */
int builtin_echo(shell_t* this, int argc, char** argv);

/** exits the shell */
int builtin_exit(shell_t* this, int argc, char** argv);

/** as the name says */
int internal_reboot(int argc, char** argv);

#endif
