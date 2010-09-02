#ifndef __LANCE_DRIVER_H__
#define __LANCE_DRIVER_H__

#include <types.h>
#include <net/ip.h>
#include <net/ether.h>

struct pci_dev;

#define LANCE_RAP 0x12 /* Register Address Port */
#define LANCE_RDP 0x10 /* Register Address Port */
#define LANCE_BDP 0x16 /* Configuration Data Port */
#define LANCE_RST 0x14 /* Reset (on read) */

#define LANCE_CSR0     0     /* Offset of CSR 0   Status and Control         */
#define LANCE_CSR1     1     /* Offset of CSR 1   Lower IADR                 */
#define LANCE_CSR2     2     /* Offset of CSR 2   Upper IADR                 */
#define LANCE_CSR3     3     /* Offset of CSR 3   Int mask, Config           */
#define LANCE_CSR4     4     /* Offset of CSR 4   Int mask, Config & status  */
#define LANCE_CSR12    12    /* Offset of CSR 12-14 MAC address              */
#define LANCE_CSR47    47    /* Offset of CSR 47  Polling interval           */
#define LANCE_CSR80    80    /* Offset of CSR 80  FIFO threshold             */
#define LANCE_CSR82    82    /* Offset of CSR 82  Bus timer                  */
#define LANCE_CSR88    88    /* Offset of CSR 88  Chip ID.                   */
#define LANCE_CSR89    89    /* Offset of CSR 89  Chip ID.                   */
#define LANCE_CSR124   124   /* Offset of CSR 124 Test Register              */

#define LANCE_IADR_LO  1
#define LANCE_IADR_HI  2

/** CSR0 bits */
#define LANCE_ERR       0x8000      /* Bit 15 - BABL, CERR, MISS OR MERR.    */
#define LANCE_BABL      0x4000      /* Bit 14 - Tx Babbled error.            */
#define LANCE_CERR      0x2000      /* Bit 13 - Tx Collision error.          */
#define LANCE_MISS      0x1000      /* Bit 12 - Rx missed error.             */
#define LANCE_MERR      0x0800      /* Bit 11 - Memory access error.         */
#define LANCE_RINT      0x0400      /* Bit 10 - Rx interrupt.                */
#define LANCE_TINT      0x0200      /* Bit 09 - Tx interrupt.                */
#define LANCE_IDON      0x0100      /* Bit 08 - Init done.                   */

#define LANCE_INTR      0x0080      /* Bit 07 - Interrupt occurred.          */
#define LANCE_INEA      0x0040      /* Bit 06 - Interrupt enable.            */
#define LANCE_RXON      0x0020      /* Bit 05 - Receiver on.                 */
#define LANCE_TXON      0x0010      /* Bit 04 - Transmitter on.              */
#define LANCE_TDMD      0x0008      /* Bit 03 - Transmit demand.             */
#define LANCE_STOP      0x0004      /* Bit 02 - Stop Lance.                  */
#define LANCE_STRT      0x0002      /* Bit 01 - Start Lance.                 */
#define LANCE_INIT      0x0001      /* Bit 00 - Initialize Lance.            */

#define IMASK      0x0300      /* PCNetISA Interrupt MASK */

void lance_driver(struct pci_dev* dev);

/* The LANCE 32-Bit initialization block, described in databook. */
struct lance_init_block {
    u_int16_t mode;
    u_int16_t tlen_rlen;
    u_int8_t  phys_addr[6];
    u_int16_t reserved;
    u_int32_t filter[2];
    /* Receive and transmit ring base, along with extra bits. */
    u_int32_t rx_ring;
    u_int32_t tx_ring;
};

/* The LANCE Rx and Tx ring descriptors. */
struct lance_rx_desc {
    u_int32_t base;
    int16_t buf_length;
    u_int16_t status;
    u_int32_t msg_length;
    u_int32_t reserved;
};

struct lance_tx_desc {
    u_int32_t base;
    int16_t length;
    u_int16_t status;
    u_int32_t misc;
    u_int32_t reserved;
};

typedef struct lance_ip_frame {
  struct ethernet_frame eth;
  u_int8_t   begin[0];

  u_int8_t   ihl:4,version:4;
  u_int8_t   tos;
  u_int16_t  tot_len;
  u_int16_t  id;
  u_int16_t  frag_off;
  u_int8_t   ttl;
  u_int8_t   protocol;
  u_int16_t  check;
  u_int8_t   src[4];
  u_int8_t   dst[4];
} __attribute__((packed))
lance_ip_frame_t;

struct lance_icmp_frame {
  struct lance_ip_frame ip;
  u_int8_t   begin[0];

  u_int8_t type;
  u_int8_t code;
  u_int16_t check;
  u_int16_t id;
  u_int16_t seq;
  char data[56];
} __attribute__((packed));

/** read a Control Status Register.
 * CSRs are readed with a two step operation. 
 * First write the address to RAP and then read
 * the value back from RDP */
u_int16_t lance_read_csr(u_int16_t base, u_int16_t address);
u_int16_t lance_read_bcr(u_int16_t base, u_int16_t address);

void lance_write_csr(u_int16_t base, u_int16_t address, u_int16_t value);
void lance_write_bcr(u_int16_t base, u_int16_t address, u_int16_t value);

void lance_ip_checksum(struct lance_ip_frame* frame);
void lance_icmp_checksum(struct lance_icmp_frame* frame);

#endif
