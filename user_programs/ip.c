#include "user_programs.h"

#include <stdio.h>
#include <unistd.h>
#include <net/ip.h>
#include <byteorder.h>

ONLY_INTERNAL;

#define CHECK_ARGS() if((i+1) == argc) { arg_error(); return 0; }

static void arg_error() {
  printf("arguments error\n");
}

int MAIN(ip, int argc, char** argv) {

  const char* version = "ip (KaOS shell builtins) 0.1\n"
    "Written by Kaos crew\n\nThis is free software\n";

  const char* usage = "Usage: ip \n"
    "Report bugs to <youknow@noonelistening.net>.\n";
  
  char *dev = "/Devices/Pcnet32/0";
  char *interface = 0;
  char *address = 0;
  ip_address_t addr = 0;
  int prom = 0;
  int do_refresh_interfaces = 0;

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
    } else if(!(strcmp(argv[i], "-d") && strcmp(argv[i], "--dev"))) {
      CHECK_ARGS();
      dev = argv[i+1];
    } else if(!(strcmp(argv[i], "-i") && strcmp(argv[i], "--interface"))) {
      CHECK_ARGS();
      interface = argv[i+1];
    } else if(!(strcmp(argv[i], "-r") && strcmp(argv[i], "--refresh"))) {
      do_refresh_interfaces = 1;
    } else if(!(strcmp(argv[i], "-a") && strcmp(argv[i], "--address"))) {
      CHECK_ARGS();
      address = argv[i+1];
    }else if(!(strcmp(argv[i], "-p") && strcmp(argv[i], "--promiscuous"))) {
      CHECK_ARGS();
      prom = *argv[i+1] == '1' ? 1 : 2;
    }
  }
  
  printf("dev = %s, interface = %s, address = %s\n", 
	 dev, interface, address);

  if(do_refresh_interfaces) {
    int fd = open("/Devices/Network/Protocols/IP", 0);
    assert(fd != -1);
    set_device_option(fd, "refresh_interfaces");
    close(fd);
  }

  if(dev) {
    int fd = open(dev, 0);
    if(fd == -1)
      return 1;

    if(address) {
      dotted_ip_address_t daddr = &addr;
      sscanf(address, "%d.%d.%d.%d", 
	     &daddr[0],
	     &daddr[1],
	     &daddr[2],
	     &daddr[3]);

      set_device_option(fd, "ip", addr);
    }

    if(prom) {
      if(prom == 1)
	set_device_option(fd, "promiscuous", 1);
      else if(prom == 2)
	set_device_option(fd, "promiscuous", 0);
    }
    close(fd);

  }

  return 0;
}
