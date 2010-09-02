#include <net/arp.h>
#include <list.h>
#include <file.h>
#include <devfs.h>
#include <malloc.h>
#include <byteorder.h>
#include <libc.h>

typedef struct {
  device_t *ip_device;
  file_t *ip_file;
  ip_interfaces_t *interfaces;
} arp_private_t;

#define private(arg) ((arp_private_t*)(arg)->private)

driver_t network_driver;
driver_t protocols_driver;
driver_t arp_driver;
device_t arp_device;

device_ops_t arp_ops = {
  .init = arp_init_device,
  .write = arp_write
};

driver_ops_t arp_driver_ops = {
  .devfs_basedir = arp_devfs_basedir,
};

static void make_eth_arp(u_int8_t* buffer, int code, 
			 ip_address_t src_ip, mac_address_t src_mac,
			 ip_address_t dst_ip, mac_address_t dst_mac);

char* arp_devfs_basedir(driver_t* this) {
  return "Network/Protocols";
}

ssize_t arp_write(device_t* this, const void* buffer, size_t len, off_t* off) {
  arp_frame_t *frame = (arp_frame_t*)buffer;
  if(len < sizeof(arp_frame_t)) {
    xprintf("packet too short %d < %d\n", len, sizeof(arp_frame_t));
  }

  if(net16_to_cpu(frame->op) == ARP_OP_REQUEST) {
    
    xprintf("arp dest ip %x\n", frame->target_ip);
    list_head_t *pos, *helper;
    ip_interfaces_t *interfaces = private(this)->interfaces;
    xprintf(" arp: interfaces %p\n", interfaces);
    xprintf(" arp: list %p\n", &interfaces->interfaces);
    xprintf(" arp: next %p\n", interfaces->interfaces.next);
    xprintf(" arp: prev %p\n", interfaces->interfaces.prev);
    
    list_for_each_safe(pos, helper, &interfaces->interfaces) {
      ip_interface_t *in = list_entry(pos, ip_interface_t, interfaces);
      xprintf(" arp: interface %p  '%s' ip %x list %p, %p\n", in, in->name, in->address,
	      &in->interfaces, pos);
      if(frame->target_ip == in->address) {
	u_int8_t mac[6];
	device_get_device_option(in->datalink, "mac", mac);
	
	// send directly the eth answer
	// TODO: use datalink methods for encapsulating arp datagram
	
	u_int8_t buffer[sizeof(ethernet_frame_t)+sizeof(arp_frame_t)];
	make_eth_arp(buffer, ARP_OP_REPLY, 
		     in->address, mac,
		     frame->sender_ip, frame->sender_eth);
	
	off_t off=0;
	uprintf("eth size %d, arp size %d, both %d\n",
		sizeof(ethernet_frame_t), sizeof(arp_frame_t),
		sizeof(ethernet_frame_t) + sizeof(arp_frame_t));
	
	uprintf("sending arp reply\n");
	int res = in->datalink->ops->write(in->datalink, buffer, sizeof(buffer), &off);
	uprintf("SENT %d\n", res);
	break;
      }
    } 
  } else if(net16_to_cpu(frame->op) == ARP_OP_REPLY) {
    uprintf("put %x into arp cache TODO\n", frame->sender_ip);
  } else {
    uprintf("unknown arp opcode %d\n", net16_to_cpu(frame->op));
  }

  return len;
}

void arp_init_device(device_t *this) {
  fnode_t *ip_fnode = file_resolve_path("/Devices/Network/Protocols/IP");
  assert(ip_fnode);
  file_t *ip_file = file_open_fnode_file(ip_fnode, 0);
  assert(ip_file);
  device_t *ip_device = devfs_device_from_file(ip_file);
  private(this)->ip_device = ip_device;
  private(this)->ip_file = ip_file;
  private(this)->interfaces = 0;
  file_get_device_option(ip_file,
			 "interfaces",
			 &private(this)->interfaces);

}

void arp_init() {
  driver_init_driver(&arp_driver);
  arp_driver.name = "ARP";
  arp_driver.ops = &arp_driver_ops;
  driver_register(&arp_driver, 0);

  device_init_device(&arp_device);
  arp_device.name = "ARP";
  arp_device.ops = &arp_ops;
  arp_device.private = malloc(sizeof(arp_private_t));

  device_attach_driver(&arp_device, &arp_driver);
  
}

///

static void make_eth_arp(u_int8_t* buffer, int code, 
			 ip_address_t src_ip, mac_address_t src_mac,
			 ip_address_t dst_ip, mac_address_t dst_mac) {

  arp_frame_t arp;
  ethernet_frame_t eth;

  arp.hard_type = cpu_to_net16(1);
  arp.prot_type = cpu_to_net16(ETHER_T_IP);
  arp.hard_size = 6;
  arp.prot_size = 4;
  arp.op = cpu_to_net16(code);
  memcpy(&arp.sender_eth, src_mac, 6);
  memcpy(&arp.target_eth, dst_mac, 6);
  arp.sender_ip = src_ip;
  arp.target_ip = dst_ip;

  memcpy(&eth.src, src_mac, 6);
  memcpy(&eth.dst, dst_mac, 6);
  eth.type = cpu_to_net16(ETHER_T_ARP);

  memcpy(buffer, &eth, sizeof(ethernet_frame_t));
  memcpy(buffer+sizeof(ethernet_frame_t), &arp, sizeof(arp_frame_t));
}
