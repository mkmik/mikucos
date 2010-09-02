#ifndef _ETHER_DEVICE_H
#define _ETHER_DEVICE_H

#include <list.h>

#define MAX_ETH_DEVICE_CHILD  80
#define NUM_PROTOCOLS   6
#define private(arg) ((ether_private_t*)(arg)->private)

typedef struct ether_private ether_private_t;

struct ether_private {
  device_t  *childs[MAX_ETH_DEVICE_CHILD];
  device_t  *proto[NUM_PROTOCOLS ];
  list_head_t my_haddrs;
  
};

#endif
