#include "user_programs.h"

#include <stdio.h>
#include <unistd.h>
#include <net/ip.h>
#include <net/arp.h>
#include <net/ether.h>
#include <byteorder.h>
#include <thread.h>
#include <stdio.h>

ONLY_INTERNAL;

static u_int8_t buff[1500];

u_int16_t make_arp(char *ffer);

static int processing_ip;

static void process_ip_thread() {
  int res;
  int eth_fd = open("/Devices/Pcnet32/0", 0);
  if(eth_fd == -1) {
    printf("Error opening net interface\n");
    return;
  }

  int arp_fd = open("/Devices/Network/Protocols/ARP", 0);
  if(arp_fd == -1) {
    printf("Error opening ARP interface\n");
    return;
  }

  int ip_fd = open("/Devices/Network/Protocols/IP", 0);
  if(ip_fd == -1) {
    printf("Error opening IP interface\n");
    return;
  }

  printf("processing IP\n");

  while(processing_ip) {
    
    memset(buff, 0, 1500);
    
    res = read(eth_fd, buff, 1500);

    assert(res != -1);

    if(res>0) {
      // decapsule ethernet
      u_int8_t* payload = buff + sizeof(ethernet_frame_t);
      int payload_size = res - sizeof(ethernet_frame_t);
      int target_layer = 0;
      
      switch(net16_to_cpu(((ethernet_frame_t*)buff)->type)) {
      case ETHER_T_IP:
	target_layer = ip_fd;
	break;
      case ETHER_T_ARP:
	target_layer = arp_fd;
	break;
      }
      
      if(target_layer)
	write(target_layer, payload, payload_size);
      else 
	xprintf("unknown layer %x\n", net16_to_cpu(((ethernet_frame_t*)buff)->type));
    }
    
  }
  close(eth_fd); 
  processing_ip = 1;
}

static void process_ip() {
  processing_ip = 1;

  char buf[10];

  thread_create(process_ip_thread, 0, 0);
  while(1) {
    printf("WAITING\n");
    fgets(buf, 10, stdin);
    printf("COMPARING '%s'\n", buf);
    if(strcmp(buf, "exit\n") == 0)
      break;
  }
  
  processing_ip = 0;
  printf("EXITING\n");
  while(!processing_ip);
  timer_delay(500);
  printf("EXITED\n");
}

int MAIN(net_test, int argc, char** argv) {

  const char* version = "net_test (KaOS shell builtins) 0.1\n"
    "C0d3d by Kaos crew \n\nThis is free software\n";

  const char* usage = "Usage: net_test\n"
    "Report bugs to <youknow@noonelistening.net>.\n";
  
  int opt_ip = 0;
  int opt_read = 0;
  int opt_write = 0;

  int i;
  int none = 1;
  for(i=1; i<argc; i++) {
    none = 0;
    if(!(strcmp(argv[i], "--help"))) {
      printf("%s", usage);
      return 0;
    } else if(!(strcmp(argv[i], "--version"))) {
      printf("%s", version);
      return 0;
    } else if(!(strcmp(argv[i], "-r") && strcmp(argv[i], "--read"))) {
      opt_read = 1;
    } else if(!(strcmp(argv[i], "-i") && strcmp(argv[i], "--ip"))) {
      opt_ip = 1;
    } else if(!(strcmp(argv[i], "-w") && strcmp(argv[i], "--write"))) {
      opt_write = 1;
    }
  }
  
  int res;
  if(opt_ip) {
    process_ip();
  } else if(opt_read) {
    int fd = open("/Devices/Pcnet32/0", 0);
    
    // read test
    while(1) {

      memset(buff, 0, 1500);

      res = read(fd, buff, 1500); 
      if(res>0) {
	char *needle = "f";
	int hi = 1500;
	int len = strlen(needle);
	int i;
	//for(i=0; i < hi; i++){
	//  if(strncmp(buff+i, needle, len)  == 0) {
	//hexdump(buff,res);
        printf("res = %d\n", res);
	    //   break;
	    //  }
	    //}	
      }      
    }    
    close(fd);

  } else if(opt_write) {
    int fd = open("/Devices/Pcnet32/0", 1);
    int ret=0;
    int i=5;
    // read test
    while (i--){
      memset(buff, 0x02, 1500);
      ret=make_arp(buff);
      
      res = write(fd, buff, ret); 
      if(res>0) {
	//	hexdump(buff,res + 16);	
      xprintf("write res=%d , ret=%d \n",res,ret);
		
      }
      
    }      
      close(fd);   
  }  
  return 0;
}

u_int16_t make_arp(char *ffer){
  
  struct pcnet32_arp pkt;  
  //ethernet_frame_t pkt;
  u_int8_t count=6;

  while(count--) pkt.dst[count]=0xff;
  count=6;
  while(count--) pkt.sender_eth[count]=0xff;
  pkt.src[0]=0x0;
  pkt.src[1]=0x50;
  pkt.src[2]=0x56;
  pkt.src[3]=0x40;
  pkt.src[4]=0x00;
  pkt.src[5]=0x5a;
  pkt.type=0x0608;
  pkt.hard_type=0x0100;
  pkt.prot_type=0x0008;
  pkt.hard_size=0x06;
  pkt.prot_size=0x04;
  pkt.op=0x100;
  memcpy(pkt.sender_eth, pkt.src, 6);
  pkt.sender_ip=0x0f00a8c0;
  memset(pkt.target_eth, 0, 6);
  pkt.target_ip=0xfa00a8c0;
  
  memcpy(ffer,&pkt,sizeof(pkt));

  //xprintf("ffer:%x\n",ffer[8]);
  //  hexdump(&pkt,16);
  return sizeof(pkt);
}
