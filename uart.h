#ifndef __UART_H__
#define __UART_H__

#include <types.h>

void uart_init();
int uart_putchar(int port, int ch);
int uart_getchar(int port);

/** returns the number of written characters */
int uart_write(int port, char* buffer, size_t len);

// private
void uart_rx_isr(int port, char ch);
void uart_tx_isr(int port);

extern struct libc_dev_ops libc_serial_ops_0;

// hack: until the is a logging subsystem 
#define uputchar(ch) dputchar(&libc_serial_ops_0, ch)
#define uputs(s) dputchar(&libc_serial_ops_0, s)
#define uprintf(args...) dprintf(&libc_serial_ops_0, args)

#endif
