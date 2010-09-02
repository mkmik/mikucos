#ifndef _PCNET32_H
#define _PCNET32_H

#include <pci.h>
#include <list.h>
#include <condvar.h>

#define private(arg) ((pcnet32_private_t*)(arg)->private)

typedef struct pcnet32_private pcnet32_private_t;

struct pcnet32_private {
  pci_dev_t *pci;
  u_int32_t ip;
  list_head_t collector_list;
  list_head_t tx_BUSY_list;
  list_head_t tx_FREE_list;
  
  condvar_t collector_condvar;
  mutex_t collector_condlock;

  u_int8_t mac[6];
  // mettero altro 
};

#endif
