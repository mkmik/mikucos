#ifndef _IP_PRIVATE_H_
#define _IP_PRIVATE_H_

#include <net/ip.h>
#include <file.h>


typedef struct {
  ip_interfaces_t* interfaces;
  file_t* icmp_file;
} ip_private_t;

#define private(arg) ((ip_private_t*)(arg)->private)

#endif
