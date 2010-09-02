#if 0
#include "user_programs.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <net/ip.h>
#include <net/arp.h>
#include <net/ether.h>
#include <byteorder.h>
#include <thread.h>
#include <stdio.h>
#include <checksum.h>

ONLY_INTERNAL;

struct buffer {
  u_int8_t dst[6];
  u_int8_t src[6];
  u_int16_t type;

  u_int8_t* ip_frame[0];
  u_int8_t   ihl:4,version:4;
  u_int8_t   tos;
  u_int16_t  tot_len;
  u_int16_t  id;
  u_int16_t  frag_off;
  u_int8_t   ttl;
  u_int8_t   protocol;
  u_int16_t  check;
  ip_address_t ip_src;
  ip_address_t ip_dst;


  u_int8_t* tcp_frame[0];
  u_int16_t tcp_sport;
  u_int16_t tcp_dport;
  u_int32_t tcp_sn;
  u_int32_t tcp_ackSn;
 
  u_int16_t res1:4;
  u_int16_t doff:4;
  u_int16_t fin:1;
  u_int16_t syn:1;
  u_int16_t rst:1;
  u_int16_t psh:1;
  u_int16_t ack:1;
  u_int16_t urg:1;
  u_int16_t ece:1;
  u_int16_t cwr:1;

  u_int16_t window;
  u_int16_t tcp_check;
  u_int16_t urg_ptr;

  u_int8_t payload_start[0];
  u_int8_t payload[500];
} PACKED;

struct buffer buffer;

int MAIN(flood, int argc, char** argv) {
  int eth_fd = open("/Devices/Pcnet32/0", 1);
  if(eth_fd == -1) {
    printf("Error opening net interface\n");
    return 0;
  }
  printf("flooding network\n");

  char *str = "PASS | karc123as\r\n";
  int header_size = buffer.payload_start - (u_int8_t*)&buffer;

  int i;
  for(i=0; i<6; i++) {
    buffer.dst[i] = 0xff;
    buffer.src[i] = 0xff;
  }
  buffer.type = cpu_to_net16(0x800);
  buffer.version = 4;
  buffer.ihl = 20/4;
  buffer.tos = 0x0;
  buffer.tot_len = cpu_to_net16(header_size + strlen(str) - 12 - 2);
  
  buffer.frag_off = 0;
  buffer.ttl = 0x40;
  buffer.protocol = 0x6;

  buffer.ip_dst = 0x0a0a0a0a;
  buffer.ip_src = 0x0b0a0a0a;

  buffer.tcp_sport = cpu_to_net16(41235);
  buffer.tcp_dport = cpu_to_net16(21);
  buffer.tcp_sn = cpu_to_net32(54335123);
  buffer.tcp_ackSn = cpu_to_net32(67243514);
  buffer.ack = 1;
  buffer.doff = 5;
  buffer.window = 40;

  ip_checksum((struct ip_frame*)buffer.ip_frame);
  buffer.tcp_check = ip_sum_calc(20 + strlen(str), (u_int16_t*)buffer.tcp_frame);

  while(1) {

    strcpy(buffer.payload, str);
    
    int res;
    printf("writing\n");
    res = write(eth_fd, &buffer, (header_size + strlen(str)));
    if(res == -1)
      printf("error\n");
    else 
      printf("ok\n");
  }
  
}

#endif
