#include <i8259.h>
#include <io.h>

static unsigned int cached_irq_mask = 0xffff;
#define cached_21 (cached_irq_mask & 0xFF)
#define cached_A1 (cached_irq_mask >> 8)


void i8259_init() {
  int auto_eoi = 0;
  int i;

  outb(0xff, 0x21); /* mask all of 8259A-1 */
  outb(0xff, 0xA1); /* mask all of 8259A-2 */
  
  /*
   * outb_p - this has to work on a wide range of PC hardware.
   */
  outb_p(0x11, 0x20);     /* ICW1: select 8259A-1 init */
  outb_p(0x20 + 0, 0x21); /* ICW2: 8259A-1 IR0-7 mapped to 0x20-0x27 */
  outb_p(0x04, 0x21);     /* 8259A-1 (the master) has a slave on IR2 */
  
  if (auto_eoi)
    outb_p(0x03, 0x21);     /* master does Auto EOI */
  else
    outb_p(0x01, 0x21);     /* master expects normal EOI */
  
  outb_p(0x11, 0xA0);     /* ICW1: select 8259A-2 init */
  outb_p(0x20 + 8, 0xA1); /* ICW2: 8259A-2 IR0-7 mapped to 0x28-0x2f */
  outb_p(0x02, 0xA1);     /* 8259A-2 is a slave on master's IR2 */
  outb_p(0x01, 0xA1);     /* (slave's support for AEOI in flat mode
			     is to be investigated) */

  //udelay(100);
  //  for(i=0; i< 0x1000000; i++);  

}

void i8259_ack(int irq) {
  if(irq & 8) {
    inb(0xA1);              /* DUMMY - (do we need this?) */
    outb(cached_A1,0xA1);
    outb(0x60+(irq&7),0xA0);/* 'Specific EOI' to slave */
    outb(0x62,0x20);        /* 'Specific EOI' to master-IRQ2 */
  } else {
    inb(0x21);              /* DUMMY - (do we need this?) */
    outb(cached_21,0x21);
    outb(0x60+irq,0x20);    /* 'Specific EOI' to master */
  }
}

static void update_i8259_mask(int irq) {
  if(irq & 8)
    outb(cached_A1, 0xA1);
  else
    outb(cached_21 & 0xFF,0x21);
}

void enable_i8259_irq(int irq) {
  cached_irq_mask &= ~(1 << irq); 
  update_i8259_mask(irq);
}

void disable_i8259_irq(int irq) {
  cached_irq_mask |= (1 << irq); 
  update_i8259_mask(irq);
}
