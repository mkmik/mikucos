#include <driver.h>
#include <device.h>
#include <malloc.h>

driver_t root_driver;
static driver_ops_t root_driver_ops;

static char* root_basedir(driver_t* this) {
  if(!this->parent)
    return this->name;

  char *parent = invoke(driver, this->parent, devfs_basedir)(this->parent);
  char *name = malloc(strlen(parent) + strlen(this->name) + 2);
  strcpy(name, parent);
  if(strlen(parent))
    strcat(name, "/");
  strcat(name, this->name);
  
  return name;
}

static void root_register_devfs(driver_t* this) {
  assert(this);
  if(this->ops)
    if(this->ops->init)
      this->ops->init(this);

  list_head_t *pos,*tmp;

  if(this == &root_driver)
    xprintf("Registering drivers \n", this->name);
  else
    xprintf("Registering driver %s\n", this->name);

  // registering devices
  device_t *device;
  list_for_each_safe(pos, tmp, &this->devices) {
    device = list_entry(pos, device_t, siblings);
    invoke(device, device, register_devfs)(device);
  }
    
  // recursively register all drivers
  driver_t *driver;
  list_for_each_safe(pos, tmp, &this->children_list) {
    driver =  list_entry(pos, driver_t, siblings_list);
    invoke(driver, driver, register_devfs)(driver);
  }
}

void driver_init() {
  driver_init_driver(&root_driver);
  
  root_driver.name = "";
  root_driver.ops = &root_driver_ops;
  root_driver.ops->devfs_basedir = root_basedir;
  root_driver.ops->register_devfs = root_register_devfs;
}

static char pname[256];

driver_t *driver_find_relative(char* path, driver_t *root) {
  if(!path)
    path = "";
  if(!*path)
    return root;

  char* pend = path;
  while(*pend != '/' && *pend != 0)
    pend++;
  
  memcpy(pname, path, pend-path);
  pname[pend-path] = 0;
  
  root = avl_find(pname, root->children, driver_t, siblings);
  if(root)
    return driver_find_relative(pend, root);
  return 0;
}

driver_t *driver_find(char* path) {
  return driver_find_relative(path, &root_driver);
}

int driver_register(driver_t *driver, char* path) {
  if(path == 0)
    path = "";
  driver_t *parent = driver_find(path);
  if(!parent)
    return -1;
  return driver_register_driver(driver, parent);
}

int driver_register_driver(driver_t *driver, driver_t *parent) {
  avl_add(&driver->siblings, &parent->children, driver_t);
  if(parent)
    list_add_tail(&driver->siblings_list, &parent->children_list);

  driver->parent = parent;

  // postpone the initialization after devfs registration
  /*  if(driver->ops)
    if(driver->ops->init) 
    driver->ops->init(driver); */
  return 0;
}

void driver_init_driver(driver_t *driver) {
  *driver =  ((driver_t){0,});
  driver->siblings = null_avl_node;
  INIT_LIST_HEAD(&driver->children_list);
  INIT_LIST_HEAD(&driver->siblings_list);
  INIT_LIST_HEAD(&driver->devices);
}

function_t driver_t_invoke_lookup(driver_t *driver, invoke_offset_t offset) {
  while(driver) {
    if(driver->ops) 
      if(*(function_t*)(((u_int8_t*)driver->ops) + offset))
	return *(function_t*)(((u_int8_t*)driver->ops) + offset);
    
    driver = driver->parent;
  }
  return invoke_panic;
}

int driver_register_devfs() {  
  driver_t *driver = driver_find("");
  invoke(driver, driver, register_devfs)(driver);
  return 1;
}
