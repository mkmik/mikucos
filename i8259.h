#ifndef __I8259__H__
#define __I8259__H__

void i8259_init();
void i8259_ack(int irq);
void enable_i8259_irq(int irq);
void disable_i8259_irq(int irq);

#endif
