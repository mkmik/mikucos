#ifndef __PCNET32_DRIVER_H__
#define __PCNET32_DRIVER_H__


#include <list.h>
#include <types.h>
//#include <device.h>
#include <pcnet32/pcnet32_private.h>

struct pci_dev;

#define PCNET32_RAP 0x12 /* Register Address Port */
#define PCNET32_RDP 0x10 /* Register Data Port */
#define PCNET32_BDP 0x16 /* Configuration Data Port */
#define PCNET32_RST 0x14 /* Reset (on read) */

#define PCNET32_CSR0     0     /* Offset of CSR 0   Status and Control         */
#define PCNET32_CSR1     1     /* Offset of CSR 1   Lower IADR                 */
#define PCNET32_CSR2     2     /* Offset of CSR 2   Upper IADR                 */
#define PCNET32_CSR3     3     /* Offset of CSR 3   Int mask, Config           */
#define PCNET32_CSR4     4     /* Offset of CSR 4   Int mask, Config & status  */
#define PCNET32_CSR15    15    /* promiscue                                    */
#define PCNET32_CSR12    12    /* Offset of CSR 12-14 MAC address              */
#define PCNET32_CSR47    47    /* Offset of CSR 47  Polling interval           */
#define PCNET32_CSR80    80    /* Offset of CSR 80  FIFO threshold             */
#define PCNET32_CSR82    82    /* Offset of CSR 82  Bus timer                  */
#define PCNET32_CSR88    88    /* Offset of CSR 88  Chip ID.                   */
#define PCNET32_CSR89    89    /* Offset of CSR 89  Chip ID.                   */
#define PCNET32_CSR124   124   /* Offset of CSR 124 Test Register              */

#define PCNET32_IADR_LO  1
#define PCNET32_IADR_HI  2

/** CSR0 bits */
#define PCNET32_ERR       0x8000      /* Bit 15 - BABL, CERR, MISS OR MERR.    */
#define PCNET32_BABL      0x4000      /* Bit 14 - Tx Babbled error.            */
#define PCNET32_CERR      0x2000      /* Bit 13 - Tx Collision error.          */
#define PCNET32_MISS      0x1000      /* Bit 12 - Rx missed error.             */
#define PCNET32_MERR      0x0800      /* Bit 11 - Memory access error.         */
#define PCNET32_RINT      0x0400      /* Bit 10 - Rx interrupt.                */
#define PCNET32_TINT      0x0200      /* Bit 09 - Tx interrupt.                */
#define PCNET32_IDON      0x0100      /* Bit 08 - Init done.                   */

#define PCNET32_INTR      0x0080      /* Bit 07 - Interrupt occurred.          */
#define PCNET32_INEA      0x0040      /* Bit 06 - Interrupt enable.            */
#define PCNET32_RXON      0x0020      /* Bit 05 - Receiver on.                 */
#define PCNET32_TXON      0x0010      /* Bit 04 - Transmitter on.              */
#define PCNET32_TDMD      0x0008      /* Bit 03 - Transmit demand.             */
#define PCNET32_STOP      0x0004      /* Bit 02 - Stop Pcnet32.                  */
#define PCNET32_STRT      0x0002      /* Bit 01 - Start Pcnet32.                 */
#define PCNET32_INIT      0x0001      /* Bit 00 - Initialize Pcnet32.            */
#define PCNET32_RING_SIZE 32          /* size of the ring buffer                 */
#define PCNET32_RING_SIZE_BITS  5      /* value of tlen/rlen                      */  
#define ETH_MTU      1500             /* mtu of ethernet                         */
#define IMASK      0x0300      /* PCNetISA Interrupt MASK */


typedef struct eth_frame_t {
  u_int8_t     payload[ETH_MTU+4];
  list_head_t  collector_list;
  u_int32_t    len;
} eth_frame_t;


typedef struct pcnet32_buff_tx_list {
  struct pcnet32_tx_desc  *desc;
  list_head_t  tx_BUFF_list;
  volatile int coled;
} pcnet32_buff_tx_list_t ;




/* The PCNET32_ 32-Bit initialization block, described in databook. */
struct pcnet32_init_block {
  u_int16_t mode;
  //    u_int16_t tlen_rlen;
  union {
    struct {
      u_int16_t rlen_res:4;
      u_int16_t rlen:4;
      u_int16_t tlen_res:4;
      u_int16_t tlen:4;
    };
    u_int16_t tlen_rlen;
  };
  u_int8_t  phys_addr[6];
  u_int16_t reserved;
  u_int32_t filter[2];
  
  /* Receive and transmit ring base, along with extra bits. */
  u_int32_t rx_ring;
  u_int32_t tx_ring;
};


/* The PCNET32_ Rx and Tx ring descriptors. */
struct pcnet32_rx_desc {
  u_int32_t base;
  int16_t buf_length;
  u_int16_t status;
  u_int16_t msg_length;
  u_int16_t res1;
  u_int32_t res2;
} __attribute__((aligned(16))) ;

struct pcnet32_tx_desc {
  u_int32_t base;
  int16_t length;
  u_int16_t status;
  u_int32_t misc;
  u_int32_t reserved;
} __attribute__((aligned(16)));




//low level func 
/** read a Control Status Register.
 * CSRs are readed with a two step operation. 
 * First write the address to RAP and then read/write
 * the value back from RDP */
u_int16_t read_csr(u_int16_t base, u_int16_t address);
u_int16_t read_bcr(u_int16_t base, u_int16_t address);

void write_csr(u_int16_t base, u_int16_t address, u_int16_t value);
void write_bcr(u_int16_t base, u_int16_t address, u_int16_t value);



// device operations func

void pcnet32_device_init(device_t* this);
void pcnet32_make_arp(device_t* this);
int pcnet32_collect(device_t *this, eth_frame_t *pkt);
int pcnet32_decollect(device_t *this, eth_frame_t *pkt);
int pcnet32_collect_tx(device_t *this, const u_int32_t *payload, u_int32_t len);
int pcnet32_decollect_tx(device_t *this, pcnet32_buff_tx_list_t *pkt);
void pcnet32_main_loop_recv(device_t *this);
void pcnet32_main_loop_send(device_t *this);

//void pcnet32_print_arp(struct pcnet32_arp pkt);
//int pcnet32_collect_tx(device_t *this, eth_frame_t *pkt);
//int pcnet32_decollect_tx(device_t *this, eth_frame_t *pkt);

#endif


