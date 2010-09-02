#ifndef _SIMPLOS_LIBC_H_
#define _SIMPLOS_LIBC_H_

#include <types.h>
#include <video.h>
#include <stdarg.h>
#include <libc_strtol.h>
#include <ctype.h>
#include <debug.h>
#include <uart.h>

/** this is a primitive way to directing
 * output to different devices within the kernel.
 * it will be changed soon*/
struct libc_dev_ops {
  int (*f_putchar)(int);
  int (*f_getchar)();
};

extern struct libc_dev_ops libc_video_ops;
extern struct libc_dev_ops libc_console_ops;
extern struct libc_dev_ops libc_serial_ops;
extern struct libc_dev_ops *libc_default_ops;

int dputchar(struct libc_dev_ops* ops, int c);
int dputs(struct libc_dev_ops* ops, const char* string);
int dprintf(struct libc_dev_ops* ops, const char* format,...);

#define putchar(ch) dputchar(libc_default_ops, ch)
#define puts(s) dputchar(libc_default_ops, s)
#define printf(args...) dprintf(libc_default_ops, args)

#define xprintf(args...) \
do {\
  printf(args);\
  uprintf(args);\
} while(0)

/** switch I/O to a vfs based console */
void use_vfs_console(int in, int out);

int sscanf(const char *str, const char *format, ...);
int fdscanf(int fd, const char *format, ...);

#define scanf(format ...) fdscanf(stdin_fd, format)
// don't know why it doesn work. use stupid macro instead
int yscanf(const char *format, ...);

void cls();
void itoa (char *buf, int base, int d);
unsigned char bcd2int(unsigned char bcd);


/** gcc optimized builtin */
int strcmp(const char *s1, const char *s2);
/** gcc optimized builtin */
int strncmp(const char *s1, const char *s2, size_t n);
/** gcc optimized builtin */
char* strcpy(char *dest, const char *str);
/** gcc optimized builtin */
char* strncpy(char *dest, const char *str, size_t n);

char *strcat(char *dest, const char *src);
char *strdup(const char *src);

char *strstr(const char *haystack, const char *needle);
char *strchr(const char *haystack, int c);
size_t strlen(const char *s);

/** gcc optimized builtin */
void *memcpy (void *__restrict __dest,
		     __const void *__restrict __src, size_t __n);
/** gcc optimized builtin */
void *memmove (void *__dest, __const void *__src, size_t __n);
/** gcc optimized builtin */  
void *memset (void *__s, int __c, size_t __n);


void setPos(int row, int col);
int getRow();
int getCol();

void hexdump(u_int8_t *data, int len);

#endif
