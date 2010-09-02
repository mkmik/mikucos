#include <pic.h>
#include <i8259.h>

void pic_init() {
  i8259_init();
}

void pic_ack(int irq) {
  i8259_ack(irq);
}
