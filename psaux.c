#include <psaux.h>
#include <i8255.h>
#include <libc.h>
#include <uart.h>
#include <hal.h>

CREATE_ISR_DISPATCHER(psaux);

void psaux_init() {
#if 0 
  i8255_write_command(KBDC_CMD_MOUSE_ENABLE);
  i8255_aux_write(KBD_AUX_ENABLE_DEV);
  i8255_set_mode(KBD_AUX_INTS_ON);
  
  set_isr(0x20 + 12, psaux_isr);
  enable_irq(2);
  enable_irq(12);
 #endif
}

void psaux_cisr() {
  uprintf(" mouse interrupt\n");
}
