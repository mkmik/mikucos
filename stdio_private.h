#ifndef _STDIO_PRIVATE_H_
#define _STDIO_PRIVATE_H_

// read stdio.h for details

#include <stdio.h>

#define BUFSIZE 256

#define ERRORINDICATOR 1
#define EOFINDICATOR 2
#define BUFINPUT 4
#define BUFLINEWISE 8
#define NOBUF 16
#define STATICBUF 32
#define FDPIPE 64

#define _IONBF 0
#define _IOLBF 1
#define _IOFBF 2

#endif
