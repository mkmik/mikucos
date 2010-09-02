// hard-coded for the 16550A chip.

#include <config.h>
#include <uart.h>
#include <libc.h>
#include <i16550A.h>
#include <hal.h>
#include <misc.h>
#include <multiboot.h>
#include <version.h>

#include <queue.h>

char_queue_t rx_queue = null_char_queue;
char_queue_t tx_queue = null_char_queue;

static int uart_initialized = 0;

static int uart_putchar_0(int ch) {
  if(ch == '\n') // hack: this should go one layer above (tty, ...)
    uart_putchar(0, '\r');
  uart_putchar(0, ch);
  return 0;  
}

struct libc_dev_ops libc_serial_ops_0 = {
  uart_putchar_0, 0
};

void uart_init() {
#if USE_UART  
  i16550A_init();
  uart_initialized = 1;

  /*  uprintf("\nMikuCOS " KERNEL_RELEASE " (AMikOS) booting from %s\n",
                multiboot_loader_name());
  uprintf("cmdline: %s\n", multiboot_cmdline());
  uprintf("\nok > "); */

#endif
}

int uart_putchar(int port, int ch) {
#if USE_UART
  if(!uart_initialized)
    return 0;

  irql_t saved_irql = hal_irql_set(IRQL_INTERRUPT);
  
  char_enqueue_nolock(&tx_queue, ch);
  i16550A_tx_ien(port, 1);

  hal_irql_restore(saved_irql);
#endif
  return 0;
}

int uart_getchar(int port) {
#if USE_UART
  if(!uart_initialized)
    return 0;
  int ch;
  irql_t saved_irql = hal_irql_set(IRQL_INTERRUPT); // really needed?

  ch = i16550A_getchar(port);

  hal_irql_restore(saved_irql);
  return ch;
#else
  return 0;
#endif
}


void uart_rx_isr(int port, char ch) {
  int s;
  if(ch == 27) {
    s = char_queue_size_nolock(&rx_queue);
  } else {
    char_enqueue_nolock(&rx_queue, ch);
    uart_putchar(port, ch); // echo
  }
}

void uart_tx_isr(int port) {
  if(char_queue_empty_nolock(&tx_queue)) {
    i16550A_tx_ien(port, 0);
//    printf(" end of tx queue\n");
  } else {
//    printf(" writing\n");
    i16550A_putchar(port, char_dequeue_nolock(&tx_queue));
  }
}

int uart_write(int port, char* buffer, size_t len) {
#if USE_UART
  if(!uart_initialized)
    return 0;

  size_t efflen, written;
  if(port != 0)
    return 0; // not yet implemented
  
  irql_t saved_irql = hal_irql_set(IRQL_INTERRUPT);

  written = efflen = MIN(char_queue_free_nolock(&tx_queue), len);
  while(efflen--)
    char_enqueue_nolock(&tx_queue, *buffer++);
  i16550A_tx_ien(port, 1);
  
  printf("written %d\n", written);
  
  hal_irql_restore(saved_irql);

  return written;
#else
  return len;
#endif
}
