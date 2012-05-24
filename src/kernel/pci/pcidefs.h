#ifndef __PCI_DEFS_H__
#define __PCI_DEFS_H__

#include "types.h"

#include "data_structures/hashtable.h"

hash_table_t device_types;

void __pci_init_defs(void);

#endif
