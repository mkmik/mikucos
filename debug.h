#ifndef __DEBUG_H_
#define __DEBUG_H_

#include <types.h>

void panic(u_int32_t err) __attribute__((noreturn));
void panicf(const char* format, ...) __attribute__((noreturn));

#define assert(expression) \
 if(!(expression)) {\
   xprintf("ASSERTION FAILED: '%s' -> %d file %s:%d\n", #expression, expression, \
	  __FILE__, __LINE__); \
   panicf("ASSERTION FAILED\n"); \
 }

#endif
