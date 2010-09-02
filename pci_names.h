#ifndef __PCI_NAMES_H__
#define __PCI_NAMES_H__

void  pci_name_device(struct pci_dev *dev);
char * pci_class_name(u_int32_t class);

#endif
