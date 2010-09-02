#include <i8255.h>
#include <hal.h>
#include <io.h>
#include <pic.h>
#include <libc.h>
#include <keyboard.h>
#include <math.h>
#include <timer.h>

CREATE_ISR_DISPATCHER(i8255);

void i8255_wait_ibs() {
  while(inb(0x64) & KBD_STAT_IBF); // spin
}

void i8255_write_ccommand(u_int8_t cmd) {
  i8255_wait_ibs();
  outb_p(cmd, 0x64);
}

void i8255_write_command(u_int8_t cmd) {
  i8255_wait_ibs();
  outb_p(cmd, 0x60);
}

void i8255_write_data(u_int8_t cmd) {
  i8255_wait_ibs();
  outb_p(cmd, 0x60);
}

void i8255_set_mode(u_int8_t val) {
  i8255_write_ccommand(KBDC_CMD_WRITE_MODE);
  outb_p(val, 0x60);
}

u_int8_t i8255_get_mode() {
  i8255_write_ccommand(KBDC_CMD_READ_MODE);
  i8255_wait_ibs();
  return inb(0x60);
}


/*********************/

void i8255_init() {
  set_isr(0x21, i8255_isr);
  
  i8255_write_command(KBD_CMD_SELECT_SCANCODE);
  i8255_write_data(2);
}

void i8255_cisr() {
  keyboard_isr(inb(0x60));
  pic_ack(1);
}

void i8255_set_led(int mask) {
  i8255_write_command(KBD_CMD_SET_LED);
  i8255_write_data(mask);
}


void i8255_enable_speaker(int on) {
  u_int8_t portb;
  disable_irq(1);
  portb = inb(0x61);
  if(on) 
    outb_p(portb | 0x3, 0x61); 
  else
    outb_p(portb & ~0x3, 0x61);
  enable_irq(1);
}

void i8255_set_freq(int freq) {
  u_int16_t latch = 1193180/freq;
  irql_t saved_irql = hal_irql_set(IRQL_INTERRUPT);
  
  outb_p(0xb6, 0x43); // timer2, latch, square, binary
  outb_p(latch & 0xFF, 0x42);
  outb_p(latch >> 8, 0x42);
  
  hal_irql_restore(saved_irql);
}

void i8255_play(int freq, int duration) {
  i8255_set_freq(freq);

  i8255_enable_speaker(1);
  delay(duration);
  i8255_enable_speaker(0);
}

void i8255_aux_write(u_int8_t val) {
  irql_t saved_irql = hal_irql_set(IRQL_INTERRUPT);

  i8255_write_ccommand(KBDC_CMD_WRITE_MOUSE);
  i8255_write_data(val);
//  i8255_wait_ibs();
  
  hal_irql_restore(saved_irql);
}
