#include <device.h>
#include <malloc.h>
#include <devfs.h>

static device_ops_t device_default_ops = {
  .init = device_default_init,
  .devfs_filename = device_default_devfs_filename,
  .register_devfs = device_default_register_devfs,
  .set_device_option = device_default_set_device_option,
  .get_device_option = device_default_get_device_option
};

void device_default_init(device_t* this) {
  assert(this->driver);
  xprintf("initializing device %s driver %s\n", this->name, this->driver->name);
}

char* device_default_devfs_filename(device_t* this) {
  if(!this->parent)
    return this->name;
  
  char *parent = invoke(device, this->parent, devfs_filename)(this->parent);
  char *name = malloc(strlen(parent) + strlen(this->name) + 2);
  strcpy(name, parent);
  if(strlen(parent))
    strcat(name, "/");
  strcat(name, this->name);
  
  return name;
}

void device_default_register_devfs(device_t* this) {
  assert(this);

  char* basedir = invoke(driver, this->driver, devfs_basedir)(this->driver);
  char* name = invoke(device, this, devfs_filename)(this);
  uprintf("REGISTERING as long name '%s'\n", name);

  char* path = malloc(strlen(name) + strlen(basedir) + 2);
  strcpy(path, basedir);
  strcat(path, "/");
  strcat(path, name);

  devfs_register_device(path, this); 

  invoke(device, this, init)(this);
}

int device_default_set_device_option(device_t* this, const char* name, va_list args) {
  //  xprintf("called device default set_device_option (%s)\n", name);
  return -1;
}

int device_default_get_device_option(device_t* this, const char* name, va_list args) {
  //  xprintf("called device default get_device_option (%s)\n", name);
  return -1;
}

void device_init_device(device_t *device) {
  *device = ((device_t){0,});
  INIT_LIST_HEAD(&device->children);
  INIT_LIST_HEAD(&device->siblings);
}

void device_attach_driver(device_t *device, driver_t *driver) {
  list_add_tail(&device->siblings, &driver->devices);
  device->parent = 0;
  device->driver = driver;
}

void device_attach_device(device_t *device, device_t *parent) {
  list_add_tail(&device->siblings, &parent->children);
  device->parent = parent;
  device->driver = parent->driver;
}

int device_get_device_option(device_t *device, const char* name, ...) {
  va_list args;
  va_start(args, name);

  assert(device);
  assert(device->ops);
  if(!device->ops->get_device_option)
    return -1;
  return device->ops->get_device_option(device, name, args);
}

function_t device_t_invoke_lookup(device_t *device, invoke_offset_t offset) {
  while(device) {
    if(device->ops)
      if(*(function_t*)(((u_int8_t*)device->ops) + offset))
	return *(function_t*)(((u_int8_t*)device->ops) + offset);

    device = device->parent;
  }
  if(!(*(function_t*)(((u_int8_t*)&device_default_ops) + offset)))
    xprintf("cannot find device method %d\n", offset/4);
  return (*(function_t*)(((u_int8_t*)&device_default_ops) + offset) ?: invoke_panic);
}
