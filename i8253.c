#include <timer.h>
#include <io.h>

#define LATCH (1193180/HZ)

void i8253_init() {
  outb_p(0x36,0x43);              /* binary, mode 3, LSB/MSB, ch 0 */
  outb_p(LATCH & 0xff , 0x40);    /* LSB */
  outb(LATCH >> 8 , 0x40);        /* MSB */
}

