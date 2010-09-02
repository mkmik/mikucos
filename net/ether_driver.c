#include <net/ether_driver.h>
#include <net/ether.h>
#include <net/ether_private.h>
#include <malloc.h>
#include <net/ip.h>


driver_t ether_driver;
device_t ether_device;
device_ops_t ether_ops = {
  .init = ether_device_init,
  // .read = ether_read,
  // .write = ether_write,
  // .set_device_option = ether_set_device_option
  // .get_device_option = ether_get_device_option
  
};

driver_ops_t ether_driver_ops = {
  .devfs_basedir = ether_devfs_basedir,
};

char* ether_devfs_basedir(driver_t* this) {
  return "Network/Protocols/Datalink";
}

void ether_init() {
  driver_init_driver(&ether_driver);
  ether_driver.name = "Ethernet";
  ether_driver.ops = &ether_driver_ops;
  driver_register(&ether_driver, 0);
  
  device_init_device(&ether_device);
  ether_device.name = "ethernet";
  ether_device.ops = &ether_ops;
  ether_device.private=malloc(sizeof(ether_private_t));
  device_attach_driver(&ether_device, &ether_driver);    
}

void ether_device_init() {

}
