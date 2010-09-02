#ifndef __i16550A_H__
#define __i16550A_H__

#define BASE(port) i16550A_ports[port].base
#define IRQ(port) i16550A_ports[port].irq

// dlab = 0
#define TX_OFFSET 0
#define RX_OFFSET 0
#define IER_OFFSET 1 
#define IIR_OFFSET 2 // interrupt identification register
#define FCR_OFFSET 2 // FIFO control register
#define LCR_OFFSET 3 // line control register
#define MCR_OFFSET 4 // modem control register
#define LSR_OFFSET 5 // line status register
#define MSR_OFFSET 6 // modem status register
#define SCRATCH_OFFSET 7 // modem status register

// dlab = 1
#define DL_OFFSET 0     // divisor latch (16 bit)
#define DL_LOW_OFFSET 0 // divisor latch low byte
#define DL_HI_OFFSET 1 // divisor latch hi byte

// LCR
#define LCR_DLAB_BIT 7
#define LCR_PARITY_BIT_0 3
#define LCR_PARITY_BIT_1 4
#define LCR_PARITY_BIT_2 5
#define LCR_STOP_BIT_BIT 2
#define LCR_WORD_LENGTH_BIT_0 0
#define LCR_WORD_LENGTH_BIT_1 1

// LSR
#define LSR_DATA_READY_BIT 0
#define LSR_EMPTY_TX_HOLDING_BIT 5
#define LSR_EMPTY_DATA_HOLDING_BIT 6

// IER
#define IER_DATA_IN_BIT 0
#define IER_DATA_OUT_BIT 1

// IIR
#define IIR_PENDING_BIT 0
#define IIR_TYPE_BIT_0 1
#define IIR_TYPE_BIT_1 2
#define IIR_TYPE_MASK 0x6

#define PARITY_N 0
#define PARITY_O 1
#define PARITY_E 2

#define WORD_LENGTH_8 3
#define WORD_LENGTH_7 2
#define WORD_LENGTH_6 1
#define WORD_LENGTH_5 0

#define MS_INTERRUPT 0 // modem status
#define TX_INTERRUPT 1
#define RX_INTERRUPT 2
#define LS_INTERRUPT 3 // line status

void i16550A_init();
char i16550A_getchar(int port);
void i16550A_putchar(int port, char ch);
void i16550A_tx_ien(int port, int en);

#endif
