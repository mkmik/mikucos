#include <pcnet32/pcnet32_driver.h>
#include <pci.h>
#include <pci_names.h>
#include <pci_ids.h>
#include <malloc.h>
#include <pcnet32/pcnet32_private.h>
#include <net/ip.h>

driver_t pcnet32_driver;
device_t pcnet32_device;
device_ops_t pcnet32_ops = {
  .init = pcnet32_device_init,
  .read = pcnet32_read,
  .write = pcnet32_write,
  .set_device_option = pcnet32_set_device_option,
  .get_device_option = pcnet32_get_device_option
};

void pcnet32_init() {
  

  driver_init_driver(&pcnet32_driver);
  pcnet32_driver.name = "Pcnet32";
  
  driver_register(&pcnet32_driver, 0);

  pci_dev_t *dev = pci_find_device(PCI_ID(PCI_VENDOR_ID_AMD, PCI_DEVICE_ID_AMD_LANCE), 0);
  printf("--------\nFOUND DEVICE %p\n--------\n", dev);
  
  device_init_device(&pcnet32_device);
  pcnet32_device.name = "0";
  pcnet32_device.ops = &pcnet32_ops;
  pcnet32_device.private=malloc(sizeof(pcnet32_private_t));
  INIT_LIST_HEAD(&(private(&pcnet32_device)->collector_list));
  INIT_LIST_HEAD(&(private(&pcnet32_device)->tx_FREE_list));
  INIT_LIST_HEAD(&(private(&pcnet32_device)->tx_BUSY_list));
  mutex_init_mutex(&private(&pcnet32_device)->collector_condlock);
  condvar_init_condvar(&private(&pcnet32_device)->collector_condvar);
  private(&pcnet32_device)->pci = dev;
  private(&pcnet32_device)->ip=0x0f00a8c0;
  device_attach_driver(&pcnet32_device, &pcnet32_driver);
  
    
}

int pcnet32_get_device_option(device_t* this, const char* name, va_list args) {
 if(strcmp(name, "mac") == 0) {
   u_int8_t* mac = va_arg(args, u_int8_t*);
   memcpy(mac, &private(this)->mac, 6);
   return 1;
 }
 return 0;
}

int pcnet32_set_device_option(device_t* this, const char* name, va_list args) {
  if(strcmp(name, "ip") == 0) {
    private(this)->ip = va_arg(args, ip_address_t);
    xprintf("pcnet32: ip set to  %x\n", private(this)->ip);
  } else if (strcmp(name,"promiscuous")==0){
    int prom = va_arg(args, int);
    xprintf("device option PROM %d\n", prom);
    write_csr(private(this)->pci->io, PCNET32_CSR0, PCNET32_STOP | PCNET32_IDON);
    write_csr(private(this)->pci->io, PCNET32_CSR15, prom ? (0x8000 | (0x3 << 7)): 0 );
    write_csr(private(this)->pci->io, PCNET32_CSR0, PCNET32_STRT | PCNET32_IDON);

  } else {
    xprintf("pcnet32: unknown device option %s\n", name);
    return -1;
  } 
  return 0;
}

ssize_t pcnet32_read(device_t* this, void* buffer, size_t len, off_t* off ) {

  /*  if (!list_empty(&private(this)->collector_list)) {
  //    uprintf("error:lista vuota\n");
  return (-1);
  }*/

  mutex_lock(&private(this)->collector_condlock);

  //  uprintf("waiting CONDVAR\n");
  while(list_empty(&private(this)->collector_list))
    condvar_wait(&private(this)->collector_condvar, &private(this)->collector_condlock);
  //  uprintf("CONDVAR signalled\n");

  eth_frame_t *my_frame = list_entry(private(this)->collector_list.next, 
				     eth_frame_t, collector_list);

  int mlen = my_frame->len - 4; // 4 bytes FCS
  if(mlen < 0) 
    mlen = 0; // hack

  if (len < mlen) {
    memcpy(buffer, my_frame->payload, len);
    *off+=len;
    uprintf("returning 0 becaus buffer len < mlen (%d < %d)\n", len, mlen);
    mutex_unlock(&private(this)->collector_condlock);
    return 0;
  }
  
  memcpy(buffer, my_frame->payload, mlen);
  *off += mlen;

  mutex_unlock(&private(this)->collector_condlock);

  pcnet32_decollect(this, my_frame);
  return mlen;
}



ssize_t pcnet32_write(device_t* this, const void* buffer, size_t len, off_t* off ) {
  printf("PCNET WRITE\n");
  if (list_empty(&private(this)->tx_FREE_list)) {
    uprintf("error: FREE lista vuota\n");
    return (-1);
  }
  
  if (len > ETH_MTU) {    
    pcnet32_collect_tx(this, buffer,ETH_MTU);
    *off+=ETH_MTU;
    return (-2);
  }
  
  return (pcnet32_collect_tx(this, buffer,len)); 
}
  
