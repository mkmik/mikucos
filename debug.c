#include <debug.h>
#include <libc.h>
#include <uart.h>
#include <hal.h>

void panic(u_int32_t err) {
  xprintf("KERNEL PANIC WITH ERR %d\n", err);
  hal_disable_interrupts();
  while (1);
}

void panicf(const char* format, ...) {
  xprintf("PANIC: %s\n", format);
  hal_disable_interrupts();
  while(1);
}
