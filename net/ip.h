#ifndef __IP_H__
#define __IP_H__

#include <types.h>
#include <device.h>
#include <list.h>
#include <avl.h>

typedef u_int32_t ip_address_t;
typedef u_int8_t *dotted_ip_address_t;

#define IP_PROTO_ICMP 0x1

typedef struct ip_frame {
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
  //  u_int8_t   src[4];
  //  u_int8_t   dst[4];
} PACKED ip_frame_t;

typedef struct {
  device_t *datalink;
  char* name;
  ip_address_t address;

  list_head_t interfaces;
  avl_node_t by_address;
} ip_interface_t;

avl_make_compare(ip_interface_t, by_address, address);
avl_make_match(ip_interface_t, address);

typedef struct {
  list_head_t interfaces;
  avl_node_t *by_address;
} ip_interfaces_t;


void ip_init();
void ip_device_init(device_t* this);
char* ip_devfs_filename(device_t* this);
char* ip_devfs_basedir(driver_t* this);

ssize_t ip_write(device_t* this, const void* buffer, size_t len, off_t* off);
int ip_set_device_option(device_t* this, const char* name, va_list args);
int ip_get_device_option(device_t* this, const char* name, va_list args);
#endif
