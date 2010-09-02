#include <hal.h>
#include <init.h>
#include <libc.h>
#include <i8259.h>
#include <io.h>
#include <idt.h>

int hal_irq_level = IRQL_KERNEL;

int getpagesize() {
  return PAGE_SIZE;
}

void set_isr(int vector, isr_t isr) {
  
  //  hal_disable_interrupts();

  // obsolete. use mal instead
#if !USE_MAL
  idt_table[vector].lowoff = (u_int16_t)((u_int32_t)isr & 0xFFFF);
  idt_table[vector].highoff = (u_int16_t)((u_int32_t)isr >> 16);
  idt_table[vector].segment = __KERNEL_CS;
  idt_table[vector].flags = 0x8E00;
#else
  mal_attachIsr(vector, isr);
#endif

  //  hal_enable_interrupts();
}

void enable_irq(int irq) {
  enable_i8259_irq(irq);
}

void disable_irq(int irq) {
  disable_i8259_irq(irq);
}

void hal_reboot() {
  while (inb (0x64) & 0x02);
  outb (0xfe, 0x64);
  // 0xd1 = turn CPU Off,  0xfe = reset
  asm ("cli;hlt");
}

void hal_halt() {
   while (inb (0x64) & 0x02);
  outb (0xd1, 0x64);
  // 0xd1 = turn CPU Off,  0xfe = reset
  asm ("cli;hlt");
}
