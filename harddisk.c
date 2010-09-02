#include <harddisk.h>
#include <device.h>
#include <partition.h>

// definitions

driver_t harddisk_driver;
driver_ops_t harddisk_ops = {
  .register_devfs = harddisk_register_devfs
};

void harddisk_register_devfs(driver_t* this) {
  assert(this->parent);
  
  // call superclass method
  harddisk_driver.parent->ops->register_devfs(this);

  // looking for partitions in every device 

  list_head_t *pos, *tmp;
  device_t *device;
  list_for_each_safe(pos, tmp, &this->devices) {
    device = list_entry(pos, device_t, siblings);
    partition_lookup(&harddisk_driver, device);
  } 
}

void harddisk_init() {
  driver_init_driver(&harddisk_driver);
  harddisk_driver.name = "Harddisk";
  harddisk_driver.ops = &harddisk_ops;
  driver_register(&harddisk_driver, "");
}
