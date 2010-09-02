#include <lance_driver.h>
#include <pci.h>
#include <libc.h>
#include <uart.h>
#include <io.h>
#include <timer.h>
#include <hal.h>
#include <net/ip.h>
#include <checksum.h>

void lance_print_status(struct pci_dev* dev);
void lance_reset(struct pci_dev* dev);
void lance_test_packet(struct pci_dev* dev);
void lance_build_packet();
static void print_mac();

struct lance_init_block init_block __attribute__((aligned(32)));

char rx_buf[4096];
char tx_buf[4096];

struct lance_icmp_frame* rx_icmp = (struct lance_icmp_frame*)rx_buf;
struct lance_icmp_frame* tx_icmp = (struct lance_icmp_frame*)tx_buf;

static struct lance_rx_desc rx_desc __attribute__((aligned(16)));
static struct lance_tx_desc tx_desc __attribute__((aligned(16)));;

u_int32_t prom[4];
u_int8_t* lance_mac = (u_int8_t*)prom;
u_int8_t phasa[] = {00,00,0xE2,0x85,0xBD,0x60};
u_int8_t eolo[] = {0xAA,00,04,00,02,04};

CREATE_ISR_DISPATCHER(lance);

u_int16_t lance_read_csr(u_int16_t base, u_int16_t address) {
  outw_p(address, base + LANCE_RAP);
  return inw(base + LANCE_RDP);
}

u_int16_t lance_read_bcr(u_int16_t base, u_int16_t address) {
  outw_p(address, base + LANCE_RAP);
  return inw(base + LANCE_BDP);
}

void lance_write_csr(u_int16_t base, u_int16_t address, u_int16_t value) {
  outw_p(address, base + LANCE_RAP);
  outw_p(value, base + LANCE_RDP);
}

void lance_write_bcr(u_int16_t base, u_int16_t address, u_int16_t value) {
  outw_p(address, base + LANCE_RAP);
  outw_p(value, base + LANCE_BDP);
}

void lance_print_status(struct pci_dev* dev) {
  u_int16_t csr0;
  
  printf("CSR0 %x:  ", csr0 = lance_read_csr(dev->io, LANCE_CSR0));
  if(csr0 & LANCE_INIT)
    printf("INIT ");
  if(csr0 & LANCE_STOP)
    printf("STOP ");
  if(csr0 & LANCE_STRT)
    printf("START ");
  if(csr0 & LANCE_IDON)
    printf("INIT_DONE ");
  if(csr0 & LANCE_RXON)
    printf("RXON ");
  if(csr0 & LANCE_TXON)
    printf("TXON ");
  if(csr0 & LANCE_INTR)
    printf("INT_R ");
  if(csr0 & LANCE_INEA)
    printf("INT_EA ");
  if(csr0 & LANCE_TDMD)
    printf("INT_TDMD ");

  if(csr0 & LANCE_TINT)
    printf("INT_TINT ");
  if(csr0 & LANCE_RINT)
    printf("INT_RINT ");
  if(csr0 & LANCE_MERR)
    printf("INT_MERR ");
  if(csr0 & LANCE_MISS)
    printf("INT_MISS ");
  if(csr0 & LANCE_CERR)
    printf("INT_CERR ");
  if(csr0 & LANCE_BABL)
    printf("INT_BABL ");
  if(csr0 & LANCE_ERR)
    printf("INT_ERR ");

  printf("\n");
}

static void print_mac() {
  int i;
  printf("Reading MAC address from ROM: %x", lance_mac[0]);
  for(i=1; i<6; i++)
    printf(":%x", lance_mac[i]);
  printf("\n");
  uprintf("Reading LANCE_MAC address from ROM: %x", lance_mac[0]);
  for(i=1; i<6; i++)
    uprintf(":%x", lance_mac[i]);
  uprintf("\n");
}

void lance_reset(struct pci_dev* dev) {
  inl(dev->io + 0x18);
}

void lance_build_packet() {
  int i;

  for(i=0; i<6; i++)
    tx_icmp->ip.eth.dst[i] = eolo[i];

  for(i=0; i<6; i++)
    tx_icmp->ip.eth.src[i] = lance_mac[i];

  tx_icmp->ip.eth.type = 0x0008; // big endian !! // IP
  tx_icmp->ip.version = 4;
  tx_icmp->ip.ihl = 5;
  tx_icmp->ip.tot_len = 0x5400;
  tx_icmp->ip.id = 0;
  tx_icmp->ip.frag_off = 0 | 0x40;
  tx_icmp->ip.ttl = 64;
  tx_icmp->ip.protocol = 1; // ICMP // UDP = 17
  tx_icmp->ip.src[0] = 192;
  tx_icmp->ip.src[1] = 168;
  tx_icmp->ip.src[2] = 0;
  tx_icmp->ip.src[3] = 66;

  tx_icmp->ip.dst[0] = 192;
  tx_icmp->ip.dst[1] = 168;
  tx_icmp->ip.dst[2] = 0;
  tx_icmp->ip.dst[3] = 250;

  tx_icmp->type = 0;
  tx_icmp->code = 0;
  tx_icmp->id = 0x3eb7;
  tx_icmp->seq = 0x024e;

  lance_ip_checksum(&tx_icmp->ip);
  lance_icmp_checksum(tx_icmp);
}

void lance_cisr() {
  printf("lance isr\n");
  uprintf("lance isr\n");
}

void lance_driver(struct pci_dev* dev) {
  int i;
  u_int16_t pci_command;

  printf("Found AMD Lance (pcnet32) ethernet interface at %x:%x.%x\n", 
	 dev->bus, dev->dev, dev->fn);
  printf("Mem %p, I/O %p irq %d\n", dev->memory, dev->io, dev->irq);

  lance_reset(dev);

  /* switch to 32bit mode */
  lance_write_bcr(dev->io, 20, 2); 

  /* read the mac address */
  for(i=0; i<4; i++) {
     prom[i] = inl(dev->io + i*4);
  }

  print_mac();
  lance_build_packet();

  /* enable bus mastering (for DMA ring access) */
  pci_bios_read_word(dev, PCI_COMMAND, &pci_command);
  pci_bios_write_word(dev, PCI_COMMAND, pci_command | PCI_COMMAND_MASTER);

  /* enable interrupt */
  set_isr(0x20 + dev->irq, lance_isr);
  enable_irq(dev->irq);

  lance_print_status(dev);

  /* initialize the "init block" */
  init_block.mode = 0; // 0=all, 1=tx, 2=rx, 3=none
  init_block.tlen_rlen = 0; // 1 ring
  for(i=0; i<6; i++)
    init_block.phys_addr[i] = lance_mac[i];
  init_block.filter[0] = 0;
  init_block.filter[1] = 0;
  init_block.rx_ring = (u_int32_t)&rx_desc;
  init_block.tx_ring = (u_int32_t)&tx_desc;

  tx_desc.base = (u_int32_t)&tx_buf;
  tx_desc.length = -(int16_t)sizeof(struct lance_icmp_frame);
  tx_desc.status = 0x0;
  tx_desc.misc = 0;

  rx_desc.base = (u_int32_t)&rx_buf;
  rx_desc.buf_length = -(int16_t)sizeof(struct lance_icmp_frame);
  rx_desc.status = 0x0000;

  lance_write_csr(dev->io, LANCE_IADR_LO, (u_int32_t)(&init_block) & 0xFFFF);
  lance_write_csr(dev->io, LANCE_IADR_HI, (u_int32_t)(&init_block) >> 16);
  lance_write_csr(dev->io, LANCE_CSR4, 0x0915);
  lance_write_csr(dev->io, LANCE_CSR0, LANCE_INIT); // initialize

  /* wait until initialized */
  i=0;
  while(i++ < 10000)
    if(lance_read_csr(dev->io, 0) & LANCE_IDON)
      goto init_done;    
  printf("timeout\n");
 init_done:

  /* continue testing */
  lance_print_status(dev);
  lance_write_csr(dev->io, LANCE_CSR0, LANCE_STRT | LANCE_IDON);
  lance_print_status(dev);

  printf("INITIALIZED\n");

  lance_test_packet(dev);
}

void lance_test_packet(struct pci_dev* dev) {
  int tx_waiting = 0;
  int rx_waiting = 0;
  int i;
  struct lance_ip_frame *rx_ip = (struct lance_ip_frame*)rx_desc.base;

  tx_desc.status = 0x300;
  tx_waiting = 0;

  rx_desc.status |= 0x8000;
  rx_waiting = 1;


  while(1) {
    // acknowledge interrupt 
    lance_write_csr(dev->io, LANCE_CSR0, lance_read_csr(dev->io, LANCE_CSR0));
    //    lance_print_status(dev);
    if(tx_waiting && !(tx_desc.status & 0x8000)) {
      //      uprintf("packed sent\n");
      tx_waiting = 0;
    }

    if(rx_waiting && !(rx_desc.status & 0x8000)) {
      xprintf("packed arrived (%x)\n", rx_desc.status);
      xprintf(" type %d\n", rx_ip->eth.type);
      xprintf(" proto %d\n", rx_ip->protocol);
      xprintf(" dst %d.%d.%d.%d\n", rx_ip->dst[0], 
	      rx_ip->dst[1],
	      rx_ip->dst[2],
	      rx_ip->dst[3]);
      rx_desc.status |= 0x8000; // receive

      if((rx_ip->dst[0] == 192 &&
	 rx_ip->dst[1] == 168 &&
	 rx_ip->dst[2] == 0 &&
	 rx_ip->dst[3] == 66 &&
	 rx_ip->eth.type == 8 &&
	 rx_ip->protocol == 1) || 1) {
	
	xprintf("sending packet\n");

	memcpy(&tx_buf, &rx_buf, sizeof(struct lance_icmp_frame));
	for(i=0;i<6;i++) {
	  tx_icmp->ip.eth.src[i] = rx_icmp->ip.eth.dst[i];
	  tx_icmp->ip.eth.dst[i] = rx_icmp->ip.eth.src[i];
	}
	for(i=0;i<4;i++) {
	  tx_icmp->ip.src[i] = rx_icmp->ip.dst[i];
	  tx_icmp->ip.dst[i] = rx_icmp->ip.src[i];
	}
	tx_icmp->type = 0; // reply
	tx_icmp->id = rx_icmp->id;
	tx_icmp->seq = rx_icmp->seq;
	lance_ip_checksum(&tx_icmp->ip);
	lance_icmp_checksum(tx_icmp);
	
	tx_desc.status |= 0x8000; // resend
	tx_waiting = 1;
      }

    }

    //    delay(100);
  }
}

/*
  tx_desc.status = 0x8300;
// Trigger an immediate send poll. 
  lance_write_csr (dev->io, LANCE_CSR0, lance_read_csr(dev->io, 0) | LANCE_TDMD);
  // acknowledge interrupts 
  lance_write_csr(dev->io, 0, lance_read_csr(dev->io, 0) & ~0x004f);
  lance_print_status(dev);  
*/


///////////////

u_int16_t lance_ip_sum_calc(int len_ip_header, u_int16_t buff[]) {
  u_int32_t sum=0;
  u_int16_t i;
    
  for (i=0;i<len_ip_header/2;i++){
    sum = sum + (u_int32_t) buff[i];
  }
	
  while (sum>>16)
    sum = (sum & 0xFFFF)+(sum >> 16);

  return ((u_int16_t) ~sum);
}

void lance_ip_checksum(struct lance_ip_frame* frame) {
  frame->check = 0;
  frame->check = lance_ip_sum_calc(20, (u_int16_t*)&frame->begin);
}

void lance_icmp_checksum(struct lance_icmp_frame* frame) {
  frame->check = 0;
  frame->check = lance_ip_sum_calc(8+128, (u_int16_t*)&frame->begin);
}
