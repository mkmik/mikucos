#include <checksum.h>

u_int16_t ip_sum_calc(int len_ip_header, u_int16_t buff[]) {
  u_int32_t sum=0;
  u_int16_t i;
    
  for (i=0;i<len_ip_header/2;i++){
    sum = sum + (u_int32_t) buff[i];
  }
	
  while (sum>>16)
    sum = (sum & 0xFFFF)+(sum >> 16);

  return ((u_int16_t) ~sum);
}

void ip_checksum(struct ip_frame* frame) {
  frame->check = 0;
  frame->check = ip_sum_calc(20, (u_int16_t*)&frame->begin);
}

void icmp_checksum(struct icmp_frame* frame) {
  frame->check = 0;
  frame->check = ip_sum_calc(8+128, (u_int16_t*)&frame->begin);
}

