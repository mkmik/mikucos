#ifndef _ICMP_PRIVATE_H_
#define _ICMP_PRIVATE_H_

#include <net/icmp.h>
#include <file.h>


typedef struct {
  ip_interfaces_t* interfaces;
  file_t* ip_file;
  file_t* arp_file;
} icmp_private_t;

#define private(arg) ((icmp_private_t*)(arg)->private)

#endif
