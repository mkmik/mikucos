#ifndef __ICMP_H__
#define __ICMP_H__

#include <types.h>
#include <device.h>
#include <net/ip.h>

#define ICMP_T_REQUEST 8
#define ICMP_T_REPLY   0

typedef struct icmp_frame {
  //  ip_frame_t;
  u_int8_t   begin[0];

  u_int8_t   ihl:4,version:4;
  u_int8_t   tos;
  u_int16_t  tot_len;
  u_int16_t  id;
  u_int16_t  frag_off;
  u_int8_t   ttl;
  u_int8_t   protocol;
  u_int16_t  check;
  ip_address_t src;
  ip_address_t dst;


  // end ip 
  //  u_int8_t begin[0];

  u_int8_t type;
  u_int8_t code;
  u_int16_t icmp_check;
  u_int16_t icmp_id;
  u_int16_t seq;
  char data[56];
} PACKED icmp_frame_t;

void icmp_init();
void icmp_device_init(device_t* this);
char* icmp_devfs_filename(device_t* this);
char* icmp_devfs_basedir(driver_t* this);

ssize_t icmp_write(device_t* this, const void* buffer, size_t len, off_t* off);

#endif
