#ifndef _STDIO_H_
#define _STDIO_H_

#include <libc.h>
#include <file_api.h>

// shitty compat 
// this one is so old to be ansi...
// FILE* functions are needed because 

#define EOF -1

// some parts cut&pasted from dietlibc (I'm tired of hot water)

struct FILE {
  int fd;
  int flags;
  unsigned int bs;      /* read: bytes in buffer */
  unsigned int bm;      /* position in buffer */
  unsigned int buflen;  /* length of buf */
  char *buf;
  struct FILE *next;    /* for fflush */
  unsigned char ungetbuf;
  char ungotten;
};
typedef struct FILE FILE;

extern FILE* stdin;
extern FILE* stdout;

extern int feof (FILE *stream);
extern int ferror(FILE *stream);
extern int fileno(FILE *stream);
extern int fgetc(FILE *stream);
extern char *fgets(char *s, int size, FILE *stream);
extern int fflush(FILE *stream);
extern FILE* fopen(const char *path, const char *mode);
extern FILE* fdopen(int fd, const char *mode);


extern int __fflush4(FILE *stream, int next);
extern int __buffered_outs(const char *s, size_t len);

#endif
