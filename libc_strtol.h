#ifndef _LIBC_STRTOL_H_
#define _LIBC_STRTOL_H_

unsigned long strtoul(const char *str, char **endptr, int base);
long strtol(const char *str, char **endptr, int base);

#endif
