#include <net/icmp.h>
#include <net/icmp_private.h>
#include <net/ether.h>
#include <malloc.h>
#include <libc.h>
#include <file.h>
#include <byteorder.h>

driver_t icmp_driver;
device_t icmp_device;

device_ops_t icmp_ops = {
  .init = icmp_device_init,
  .write = icmp_write,
  //  .set_device_option = icmp_set_device_option,
  //  .get_device_option = icmp_get_device_option
};

driver_ops_t icmp_driver_ops = {
  .devfs_basedir = icmp_devfs_basedir,
};

static void make_eth_icmp_reply(device_t* this, u_int8_t* buffer, icmp_frame_t *orig);

char* icmp_devfs_basedir(driver_t* this) {
  return "Network/Protocols";
}

ssize_t icmp_write(device_t* this, const void* buffer, size_t len, off_t* off) {
  icmp_frame_t *frame = (icmp_frame_t*)buffer;
  
  xprintf("GOT ICMP FRAME\n");
  u_int8_t buf[sizeof(ethernet_frame_t)+sizeof(icmp_frame_t)];
  switch(frame->type) {
  case ICMP_T_REQUEST:    
    make_eth_icmp_reply(this, buf, frame);

    break;
  }
  return len;
}

void icmp_init() {
  driver_init_driver(&icmp_driver);
  icmp_driver.name = "ICMP";
  icmp_driver.ops = &icmp_driver_ops;
  driver_register(&icmp_driver, 0);

  device_init_device(&icmp_device);
  icmp_device.name = "ICMP";
  icmp_device.ops = &icmp_ops;
  icmp_device.private = malloc(sizeof(icmp_private_t));

  device_attach_driver(&icmp_device, &icmp_driver);
}

void icmp_device_init(device_t *this) {
  uprintf("ICMP INIT\n");
  fnode_t *ip_fnode = file_resolve_path("/Devices/Network/Protocols/IP");
  assert(ip_fnode);
  file_t *ip_file = file_open_fnode_file(ip_fnode, 0);
  assert(ip_file);
  private(this)->ip_file = ip_file;
  private(this)->interfaces = 0;
  file_get_device_option(ip_file,
			 "interfaces",
			 &private(this)->interfaces);

  fnode_t *icmp_fnode = file_resolve_path("/Devices/Network/Protocols/ICMP");
  assert(icmp_fnode);
  file_t *icmp_file = file_open_fnode_file(icmp_fnode, 0);
  assert(icmp_file);

  file_set_device_option(ip_file,
			 "icmp_file",
			 icmp_file);
  
  // only needed until whe have a working layered sending (SAP)
  fnode_t *arp_fnode = file_resolve_path("/Devices/Network/Protocols/ARP");
  assert(arp_fnode);
  file_t *arp_file = file_open_fnode_file(arp_fnode, 0);
  assert(arp_file);
  private(this)->arp_file = arp_file;
}

static void make_eth_icmp_reply(device_t* this, u_int8_t* buffer, icmp_frame_t *orig) {
  icmp_frame_t new = *orig;
  ethernet_frame_t eth;

  new.type = ICMP_T_REPLY;
  new.src = orig->dst;
  new.dst = orig->src;
  
  eth.type = cpu_to_net16(ETHER_T_IP);

  list_head_t *pos, *helper;
  ip_interfaces_t *interfaces = private(this)->interfaces;
    
  list_for_each_safe(pos, helper, &interfaces->interfaces) {
    ip_interface_t *in = list_entry(pos, ip_interface_t, interfaces);
    mac_address_t mac;
    device_get_device_option(in->datalink, "mac", &mac);
    memcpy(eth.dst, mac, sizeof(mac_address_t));

    break;
  }
  
  memcpy(buffer, &eth, sizeof(ethernet_frame_t));
  memcpy(buffer+sizeof(ethernet_frame_t), &new, sizeof(icmp_frame_t));
}
