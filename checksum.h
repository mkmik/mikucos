#ifndef __CHECKSUM_H__
#define __CHECKSUM_H__

#include <net/ip.h>
#include <net/icmp.h>

void ip_checksum(struct ip_frame* frame);
void icmp_checksum(struct icmp_frame* frame);

#endif
