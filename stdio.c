#include <stdio_private.h>
#include <file_api.h>
#include <malloc.h>

#include <debug.h>

static char __stdin_buf[BUFSIZE];
static FILE __stdin = {
  .fd = 0,
  .flags = BUFINPUT|STATICBUF,
  .bs = 0, .bm = 0,
  .buflen = BUFSIZE,
  .buf = __stdin_buf,
  .next = 0,
  .ungetbuf = 0,
  .ungotten = 0
};

static char __stdout_buf[BUFSIZE];
static FILE __stdout = {
  .fd = 1,
  .flags = BUFLINEWISE|STATICBUF,
  .bs = 0, .bm = 0,
  .buflen = BUFSIZE,
  .buf = __stdout_buf,
  .next = 0,
  .ungetbuf = 0,
  .ungotten = 0
};


FILE *stdin = &__stdin;
FILE *stdout = &__stdout;
FILE *__stdio_root;

int fgetc_unlocked(FILE *stream) {
  unsigned char c;
  if (stream->ungotten) {
    stream->ungotten = 0;
    return stream->ungetbuf;
  }
  if (feof(stream))
    return EOF;
  if (__fflush4(stream, BUFINPUT)) 
    return EOF;
  if (stream->bm >= stream->bs) {
    int len = read(stream->fd,stream->buf,stream->buflen);
    if (len==0) {
      stream->flags |= EOFINDICATOR;
      return EOF;
    } else if (len < 0) {
      stream->flags |= ERRORINDICATOR;
      return EOF;
    }
    stream->bm = 0;
    stream->bs = len;
  }
  c = stream->buf[stream->bm];
  ++stream->bm;
  return c;
}

int fgetc(FILE* stream) __attribute__((weak,alias("fgetc_unlocked")));

char *fgets(char *s, int size, FILE *stream) {
  char *orig = s;
  int l;
  for (l=size; l>1;) {
    int c = fgetc(stream);
    if (c == EOF) break;
    *s = c;
    ++s;
    --l;
    if (c == '\n') break;
  }
  if (l == size || ferror(stream))
    return 0;
  *s=0;
  return orig;
}

int fileno(FILE *stream) {
  return stream->fd;
}

int ferror(FILE *stream) {
  return (stream->flags&ERRORINDICATOR);
}

int feof(FILE *stream) {
  /* yuck!!! */
  if(stream->ungotten) return 0;
  return stream->flags & EOFINDICATOR;
}

int fflush(FILE *stream) {
  if(stream == 0) {
    panicf("not yet implemented (fflush(0))");
    /*
    int res;
    FILE *f;
    __fflush_stdin();
    __fflush_stdout();
    __fflush_stderr(); 
    for(res=0, f=__stdio_root; f; f=f->next)
      if(fflush(f))
        res=-1;
    return res; */
  }
//  if (stream->flags&NOBUF) return 0;
  if(stream->flags & BUFINPUT) {
    register int tmp;
    if ((tmp=stream->bm-stream->bs)) {
      panicf("not yet implemented (fflush lseek)");
      //      lseek(stream->fd,tmp,SEEK_CUR);
    }
    stream->bs=stream->bm=0;
  } else {
    if (stream->bm && write(stream->fd,stream->buf,stream->bm)!=(int)stream->bm) {
      stream->flags|=ERRORINDICATOR;
      return -1;
    }
    stream->bm=0;
  }
  return 0;
}


int __fflush4(FILE *stream, int next) {
  /*  if (!__stdio_atexit) { // not implemented in kaos
    __stdio_atexit=1;
    atexit(__stdio_flushall);
    }*/
  if ((stream->flags & BUFINPUT) != next) {
    int res = fflush(stream);
    stream->flags = (stream->flags & ~BUFINPUT) | next;
    return res;
  }
  if (stream->fd == 0)
    fflush(stdout);
  if (stream->fd == 1)
    fflush(stdout);
  return 0;
}

FILE* __stdio_init_file(int fd,int closeonerror) 
     __attribute__((weak,alias("__stdio_init_file_nothreads")));

FILE *fdopen(int filedes, const char *mode) {
  //  int f=0;      /* O_RDONLY, O_WRONLY or O_RDWR */

  //  f=__stdio_parse_mode(mode);
  if (filedes < 0) { 
    // errno=EBADF;  // TODO implement errno
    return 0; 
  }
  return __stdio_init_file(filedes, 0);
}

FILE *fopen(const char *path, const char *mode) {
  //  int f=0;      /* O_RDONLY, O_WRONLY or O_RDWR */
  int fd;

  //  f=__stdio_parse_mode(mode);
  //  if ((fd=open(path,f,0666)) < 0)
  if ((fd = open(path, 0)) < 0)
    return 0;
  return __stdio_init_file(fd, 1);
}


FILE* __stdio_init_file_nothreads(int fd,int closeonerror) {
  FILE *tmp = malloc(sizeof(FILE));
  if (!tmp) goto err_out;
  tmp->buf = (char*)malloc(BUFSIZE);
  if (!tmp->buf) {
    free(tmp);
  err_out:
    if (closeonerror) close(fd);
    //    errno = ENOMEM;
    return 0;
  }
  tmp->fd = fd;
  tmp->bm = 0;
  tmp->bs = 0;
  tmp->buflen = BUFSIZE;

  tmp->flags = 0;
  /*  {
    struct stat st;
    fstat(fd,&st);
    tmp->flags = (S_ISFIFO(st.st_mode))?FDPIPE:0;
    }*/
  /*  tmp->popen_kludge = 0;
  if (__stdio_atexit == 0) {
    __stdio_atexit = 1;
    atexit(__stdio_flushall);
    } */
  tmp->next = __stdio_root;
  __stdio_root = tmp;
  tmp->ungotten = 0;
  return tmp;
}
