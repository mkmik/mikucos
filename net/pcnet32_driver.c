#include <net/pcnet32_driver.h>
#include <pci.h>
#include <libc.h>
#include <uart.h>
#include <io.h>
#include <timer.h>
#include <hal.h>
#include <net/ip.h>
#include <checksum.h>
#include <device.h>
#include <net/arp.h>
#include <byteorder.h>
#include <thread.h>
#include <net/arp.h>
#include <libc.h>

void pcnet32_print_status(struct pci_dev* dev);
void pcnet32_reset(struct pci_dev* dev);
void pcnet32_print_arp(struct pcnet32_arp *pkt);
void pcnet32_make_arp(device_t* this);
void pcnet32_ether_test(device_t* this);

static void print_mac();

struct pcnet32_init_block init_block __attribute__((aligned(32)));

char rx_buf[4096];
char tx_buf[4096];


static struct pcnet32_rx_desc rx_desc[PCNET32_RING_SIZE] __attribute__((aligned(16)));
static struct pcnet32_tx_desc tx_desc[PCNET32_RING_SIZE] __attribute__((aligned(16)));

static eth_frame_t f_buff_rx[PCNET32_RING_SIZE];
//static eth_frame_t f_buff_tx[PCNET32_RING_SIZE];
static pcnet32_buff_tx_list_t tx_buffer[PCNET32_RING_SIZE];



u_int32_t mymac[4];

u_int8_t* mac = (u_int8_t*)mymac;


CREATE_ISR_DISPATCHER(pcnet32);

// low level function
u_int16_t read_csr(u_int16_t base, u_int16_t address) {
  outw_p(address, base + PCNET32_RAP);
  return inw(base + PCNET32_RDP);
}

u_int16_t read_bcr(u_int16_t base, u_int16_t address) {
  outw_p(address, base + PCNET32_RAP);
  return inw(base + PCNET32_BDP);
}

void write_csr(u_int16_t base, u_int16_t address, u_int16_t value) {
  outw_p(address, base + PCNET32_RAP);
  outw_p(value, base + PCNET32_RDP);
}

void write_bcr(u_int16_t base, u_int16_t address, u_int16_t value) {
  outw_p(address, base + PCNET32_RAP);
  outw_p(value, base + PCNET32_BDP);
}

void pcnet32_print_status(struct pci_dev* dev) {
  u_int16_t csr0;
  
  printf("CSR0 %x:  ", csr0 = read_csr(dev->io, PCNET32_CSR0));
  if(csr0 & PCNET32_INIT)
    printf("INIT ");
  if(csr0 & PCNET32_STOP)
    printf("STOP ");
  if(csr0 & PCNET32_STRT)
    printf("START ");
  if(csr0 & PCNET32_IDON)
    printf("INIT_DONE ");
  if(csr0 & PCNET32_RXON)
    printf("RXON ");
  if(csr0 & PCNET32_TXON)
    printf("TXON ");
  if(csr0 & PCNET32_INTR)
    printf("INT_R ");
  if(csr0 & PCNET32_INEA)
    printf("INT_EA ");
  if(csr0 & PCNET32_TDMD)
    printf("INT_TDMD ");
  
  if(csr0 & PCNET32_TINT)
    printf("INT_TINT ");
  if(csr0 & PCNET32_RINT)
    printf("INT_RINT ");
  if(csr0 & PCNET32_MERR)
    printf("INT_MERR ");
  if(csr0 & PCNET32_MISS)
    printf("INT_MISS ");
  if(csr0 & PCNET32_CERR)
    printf("INT_CERR ");
  if(csr0 & PCNET32_BABL)
    printf("INT_BABL ");
  if(csr0 & PCNET32_ERR)
    printf("INT_ERR ");
  
  printf("\n");
}

static void print_mac() {
  int i;
  xprintf("Reading MAC address from ROM: %x", mac[0]);
  for(i=1; i<6; i++)
    xprintf(":%x", mac[i]);
  xprintf("\n");
}

void pcnet32_reset(struct pci_dev* dev) {
  inl(dev->io + 0x18);
}

void pcnet32_cisr() {
  printf("pcnet32 isr\n");
  uprintf("pcnet32 isr\n");
}


// init function 

void pcnet32_device_init(device_t* this){
  
  pci_dev_t  *dev;
  int i;
  u_int16_t pci_command;
  dev = private(this)->pci;
  
  uprintf("AMD Pcnet32 ethernet interface at %x:%x.%x\n", dev->bus, dev->dev, dev->fn);
  uprintf("Mem %p, I/O %p irq %d\n", dev->memory, dev->io, dev->irq);
  
  pcnet32_reset(dev);
  
  /* 32bit mode */
  write_bcr(dev->io, 20, 2); 
  
  /* the card  mac address */
  for(i=0; i<4; i++) {
    mymac[i] = inl(dev->io + i*4);
    memcpy(&private(this)->mac, mymac, 6);
  }
  
  print_mac();
  
  
  /* enable bus mastering (for DMA ring access) */
  pci_bios_read_word(dev, PCI_COMMAND, &pci_command);
  pci_bios_write_word(dev, PCI_COMMAND, pci_command | PCI_COMMAND_MASTER);
  
  /* abilitiamo gli interreupt */
  set_isr(0x20 + dev->irq, pcnet32_isr);
  enable_irq(dev->irq);

  pcnet32_print_status(dev);
  
  /* mettiamo su l' "init block" */
  
  init_block.mode = 0x000; // 0=all, 1=tx, 2=rx, 3=none
  init_block.tlen_res = 0;
  init_block.rlen_res = 0;
  init_block.tlen = PCNET32_RING_SIZE_BITS; // length of trasmission buffer ring
  init_block.rlen = PCNET32_RING_SIZE_BITS; // length of recv  buffer ring
  
  for(i=0; i<6; i++)
    init_block.phys_addr[i] = mac[i]; // paddr: mac address in the init block 
  
  init_block.filter[0] = 0;          // filter for multicast etc
  init_block.filter[1] = 0;
  init_block.rx_ring = (u_int32_t)rx_desc; // init recv descriptor ring buffer addr
  init_block.tx_ring = (u_int32_t)tx_desc; // init send descriptor ring buffer addr
  
  
  for(i=0;i<PCNET32_RING_SIZE;i++){
    //tx_desc[i].base = (u_int32_t)&f_buff_tx[i];old system
    


    tx_desc[i].length = -(int16_t)(ETH_MTU);
    tx_desc[i].status = 0x300;
    tx_desc[i].misc = 0;
    
    rx_desc[i].base = (u_int32_t)&f_buff_rx[i];
    rx_desc[i].buf_length = -(int16_t)(sizeof(f_buff_rx->payload));
    rx_desc[i].status = 0x8000;

    // tx buffering init
    tx_buffer[i].desc=&tx_desc[i];// inizializzo il buffer di puntatori ai descrittori di trasmissione 
    tx_buffer[i].coled=0;// test 
    list_add_tail(&tx_buffer[i].tx_BUFF_list, &private(this)->tx_FREE_list); // viene riempita la lista con tutti i descrittori
    
    
    INIT_LIST_HEAD(&f_buff_rx[i].collector_list);
  }
  

  write_csr(dev->io, PCNET32_IADR_LO, (u_int32_t)(&init_block) & 0xFFFF);
  write_csr(dev->io, PCNET32_IADR_HI, (u_int32_t)(&init_block) >> 16);
  write_csr(dev->io, PCNET32_CSR4, 0x0915);
  write_csr(dev->io, PCNET32_CSR0, PCNET32_INIT); // initialize
  
  /* aspettiamo porcodio  */
  i=0;
  while(i++ < 10000) 
    if(read_csr(dev->io, 0) & PCNET32_IDON) 
      goto init_done;    
  printf("timeout\n");
   panic("TIMEOUT");
 init_done:
  
  write_csr(dev->io, PCNET32_CSR0, PCNET32_STRT | PCNET32_IDON);
  
  pcnet32_print_status(dev);
  
  xprintf("AMD pcnet32: I'm ready to run on the net\n");
  
  for(i = 0; i < PCNET32_RING_SIZE; i++) 
    tx_desc[i].status = 0x300;
  
  for(i = 0; i < PCNET32_RING_SIZE; i++)  rx_desc[i].status |= 0x8000;
  
  thread_create(pcnet32_main_loop_recv, this, 0);
  thread_create(pcnet32_main_loop_send, this, 0);
  //  pcnet32_ether_test(this);
  
  
}

// recv functions
int pcnet32_collect(device_t *this, eth_frame_t *pkt){
  mutex_lock(&private(this)->collector_condlock);

  list_add_tail(&pkt->collector_list, &private(this)->collector_list);

  condvar_broadcast(&private(this)->collector_condvar);
  mutex_unlock(&private(this)->collector_condlock);
  return 0;
}


int pcnet32_decollect(device_t *this, eth_frame_t *pkt){
  
  list_del(&pkt->collector_list);
  
  INIT_LIST_HEAD(&pkt->collector_list);
  // dealloc the descriptor corresponding to the pkt ring <buffer
  rx_desc[pkt - f_buff_rx].status |= 0x8000;
  
  return 0;
}

//   send functions
int pcnet32_collect_tx(device_t *this, const u_int32_t *payload, u_int32_t len){
  xprintf("PCNET32 collect tx\n");

  pcnet32_buff_tx_list_t  *pkt = list_entry(private(this)->tx_FREE_list.next,  
					    pcnet32_buff_tx_list_t , tx_BUFF_list);
  // viene prelevato il primo desc libero dalla free list  
  
  list_del(&pkt->tx_BUFF_list); // viene eliminato dalla free list 
  
  pkt->desc->base = (u_int32_t)payload;    // viene passato il puntatore al contenuto e la sua relativa lunghezza
  pkt->desc->length = -(u_int16_t)len;
  
  pkt->coled=1;
  list_add_tail(&pkt->tx_BUFF_list, &private(this)->tx_BUSY_list); // il pacchetto viene inserito nel buffer

  // non decolletta e spedisce il messaggio
  //  uprintf("user thread: while coled (%p)\n", pkt);
  xprintf("waiting coled\n");
  while(pkt->coled);
  xprintf("coled is 0\n");

  return len;
}

int pcnet32_decollect_tx(device_t *this, pcnet32_buff_tx_list_t *pkt){
  
  assert(!list_empty(&private(this)->tx_BUSY_list));
  list_del(&pkt->tx_BUFF_list); // elimina il pkt dalla BUSY list

  
  //INIT_LIST_HEAD(&pkt->tx_BUFF_list);
  // dealloc the descriptor corresponding to the pkt ring <buffer
  
  //tx_desc[pkt - f_buff_tx].status |= 0x8000;  old system 
  pkt->desc->status |= 0x8000;
  xprintf("LOW packet send wait desc %p stat %x\n", pkt->desc, pkt->desc->status);
  while (pkt->desc->status & 0x8000);
  xprintf("LOW packet sent\n");

  list_add_tail(&pkt->tx_BUFF_list, &private(this)->tx_FREE_list); 
  pkt->coled=0;
  return 0;
}


// recv polling loop 

void pcnet32_main_loop_recv(device_t *this){
  int i;
  int rx_waiting=1;

  while (1) {  
    // condvar wait for interrupt
    //   .... wile(condition) condvar_wait(blabla);;;;;; ....
    // interrupt arrived

    for(i = 0; i < PCNET32_RING_SIZE; i++) {
      write_csr(private(this)->pci->io, PCNET32_CSR0,read_csr(private(this)->pci->io, PCNET32_CSR0));
          
      if(list_empty(&f_buff_rx[i].collector_list)) {	
	if(rx_waiting && !(rx_desc[i].status & 0x8000)) {
	  f_buff_rx[i].len = rx_desc[i].msg_length;
	  uprintf("LOW got packet\n");
	  pcnet32_collect(this, &f_buff_rx[i]);	  
	}       	
      }      
    }
  }
}

// send polling loop 

void pcnet32_main_loop_send(device_t *this){
  int i;
  int tx_waiting=1; 
  while(1){
    
    write_csr(private(this)->pci->io, PCNET32_CSR0,read_csr(private(this)->pci->io, PCNET32_CSR0));
 
    if(!list_empty(&private(this)->tx_BUSY_list)) {
      pcnet32_buff_tx_list_t *pkt = list_entry(private(this)->tx_BUSY_list.next,  
					       pcnet32_buff_tx_list_t , tx_BUFF_list);
      uprintf("main loop send: chiamando decollect(%p, desc = %p)\n", pkt, pkt->desc);
      pcnet32_decollect_tx(this, pkt);
    }
    
    /*    for(i = 0; i < PCNET32_RING_SIZE; i++) {
      if(!list_empty(&private(this)->tx_BUSY_list)){
       	if(tx_waiting && tx_buffer[i].coled){
	  pcnet32_decollect_tx(this, &tx_buffer[i]);
	  
	}
	
      } //else uprintf("busy list vuota\n");
      } */
  }
}
