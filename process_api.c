#include <process_api.h>
#include <process.h>

char *getcwd(char *buf, size_t size) {
  int tmp;
  if ((tmp = __syscall_getcwd(buf, size)) < 0)
    return 0;
  buf[tmp] = 0;
  return buf;
}

int __syscall_getcwd(char *buf, size_t size) {
  char *cwd = process_current()->cwd;
  if(strlen(cwd) < size) {    
    strcpy(buf, cwd);
    return strlen(cwd);
  }
  return 0;
}
