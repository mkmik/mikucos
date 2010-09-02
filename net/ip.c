#include <net/ip.h>
#include <net/ip_private.h>
#include <net/arp.h>
#include <net/icmp.h>
#include <file.h>
#include <devfs.h>
#include <malloc.h>
#include <byteorder.h>

driver_t network_driver;
driver_t protocols_driver;
driver_t ip_driver;
device_t ip_device;

device_ops_t ip_ops = {
  .init = ip_device_init,
  .write = ip_write,
  .set_device_option = ip_set_device_option,
  .get_device_option = ip_get_device_option
};

driver_ops_t ip_driver_ops = {
  .devfs_basedir = ip_devfs_basedir,
};

static void init_interfaces(device_t *this);

char* ip_devfs_basedir(driver_t* this) {
  return "Network/Protocols";
}

ssize_t ip_write(device_t* this, const void* buffer, size_t len, off_t* off) {
  ip_frame_t *frame = (ip_frame_t*)buffer;

  // don't decapsule IP!
  //  u_int8_t* payload = buffer + sizeof(ip_frame_t);
  //  int payload_size = len - sizeof(ip_frame_t);
  u_int8_t* payload = buffer;
  int payload_size = len;
  file_t *target_layer = 0;
  
  switch(frame->protocol) {
  case IP_PROTO_ICMP:
    target_layer = private(this)->icmp_file;
    break;
  }
  
  if(target_layer)
    file_write(target_layer, payload, payload_size);
  else  
    xprintf("GOT IP FRAME unknown protocol %x\n", frame->protocol);
    
  return len;
}

int ip_get_device_option(device_t* this, const char* name, va_list args) {
  if(strcmp(name, "interfaces") == 0) {
    ip_interfaces_t **res = va_arg(args, ip_interfaces_t**);
    *res = private(this)->interfaces;
  } else {
    xprintf("ip: unknown device option %s\n", name);
    return -1;
  } 
  return 0;
}

int ip_set_device_option(device_t* this, const char* name, va_list args) {
  if(strcmp(name, "refresh_interfaces") == 0) {
    init_interfaces(this);
  } if(strcmp(name, "icmp_file") == 0) {
    file_t *icmp_file = va_arg(args, file_t*);
    private(this)->icmp_file = icmp_file;
  } else {
    xprintf("ip: unknown device option %s\n", name);
    return -1;
  } 
  return 0;
}

void ip_init() {
  driver_init_driver(&ip_driver);
  ip_driver.name = "IP";
  ip_driver.ops = &ip_driver_ops;
  driver_register(&ip_driver, 0);

  device_init_device(&ip_device);
  ip_device.name = "IP";
  ip_device.ops = &ip_ops;
  ip_device.private = malloc(sizeof(ip_private_t));
  ip_interfaces_t *interfaces = malloc(sizeof(ip_interfaces_t));
  xprintf("ip: interfaces %p\n", interfaces);
  INIT_LIST_HEAD(&interfaces->interfaces);
  interfaces->by_address = 0;
  private(&ip_device)->interfaces = interfaces;

  //init_interfaces(&ip_device);

  device_attach_driver(&ip_device, &ip_driver);

  arp_init();
  icmp_init();
}

void ip_device_init(device_t* this) {
  /*  fnode_t *icmp_fnode = file_resolve_path("/Devices/Network/Protocols/ICMP");
  assert(icmp_fnode);
  file_t *icmp_file = file_open_fnode_file(icmp_fnode, 0);
  assert(icmp_file);
  private(&ip_device)->icmp_file = icmp_file; */

  private(&ip_device)->icmp_file = 0; 
}


static char dummy1[100] = {0,};
static ip_interface_t en0;
static char dummy2[100] = {0,};

void init_interfaces(device_t *this) {
  xprintf("INITIALIZING INTERFACES\n");

  ip_interfaces_t *interfaces = private(this)->interfaces;
  //  ip_interface_t *in = malloc(sizeof(ip_interface_t));
  ip_interface_t *in = &en0;
  in->by_address = null_avl_node;
  
  in->name = "en0";

  fnode_t *fnode = file_resolve_path("/Devices/Pcnet32/0");
  assert(fnode);
  file_t *file = file_open_fnode_file(fnode, 0);
  assert(file);
  in->datalink = devfs_device_from_file(file);
  assert(in->datalink);
  in->address = 0x1000a8c0;

  list_add(&in->interfaces, &interfaces->interfaces);
  //  avl_add(&in->by_address, &interfaces->by_address, ip_interface_t);
}
