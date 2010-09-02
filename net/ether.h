#ifndef __ETHER_H__
#define __ETHER_H__

#define ETHER_T_IP 0x0800
#define ETHER_T_ARP 0x0806

typedef u_int8_t mac_address_t[6];

typedef struct ethernet_frame {
  u_int8_t dst[6];
  u_int8_t src[6];
  u_int16_t type;
} PACKED ethernet_frame_t;

#endif
