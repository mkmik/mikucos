#ifndef __PCNET32_ARP_H__
#define __PCNET32_ARP_H__

#include <net/ip.h>
#include <net/ether.h>

struct pcnet32_arp {

  //  struct ethernet_frame;
  u_int8_t dst[6];
  u_int8_t src[6];
  u_int16_t type;
  union{
    struct{
      u_int16_t hard_type;
      u_int16_t prot_type;
      u_int8_t  hard_size;
      u_int8_t  prot_size;
      u_int16_t op;
      u_int8_t  sender_eth[6];
      u_int32_t  sender_ip;
      u_int8_t  target_eth[6];
      u_int32_t  target_ip;
      char pad[18];
    } PACKED;
    //    char ip_payload[60-sizeof(struct ethernet_frame)];
  };
} __attribute__((packed));

#define ARP_OP_REQUEST 1
#define ARP_OP_REPLY 2

typedef struct arp_frame {
  u_int16_t hard_type;
  u_int16_t prot_type;
  u_int8_t  hard_size;
  u_int8_t  prot_size;
  u_int16_t op;
  u_int8_t  sender_eth[6];
  u_int32_t sender_ip;
  u_int8_t  target_eth[6];
  u_int32_t target_ip;
} PACKED arp_frame_t;

void arp_init();
void arp_init_device(device_t *this);
char* arp_devfs_filename(device_t* this);
char* arp_devfs_basedir(driver_t* this);

ssize_t arp_write(device_t* this, const void* buffer, size_t len, off_t* off);

#endif
