#ifndef __PCI_H__
#define __PCI_H__

#include "headers.h"

#define PCI_CONFIG_SPACE_PORT 0xCF8
#define PCI_CONFIG_DATA_PORT  0xCFC
#define ENABLE_PCI_CONFIG_SPACE 0x80000000

#define PCI_CLASS_REVISION 0x08 /* Highest 24-bits are class, low and revision */
#define PCI_HEADER_TYPE 0x0e /* 8-bits */

// The format of CONFIG_ADDRESS is the following:
// bus << 16  |  device << 11  |  function <<  8  |  offset

// 31 	        30 - 24 	23 - 16 	15 - 11 	    10 - 8 	            7 - 2 	            1 - 0 
// Enable Bit 	Reserved 	Bus Number 	Device Number 	Function Number 	Register Number 	00 

// Useful wiki
// http://en.wikipedia.org/wiki/Input/Output_Base_Address

/* Initialize the pci devices
 */
void __pci_init(void);

Uint32 __pci_config_read_long(Uint8 bus, Uint8 device, Uint8 function, Uint8 offset);
Uint16 __pci_config_read_short(Uint8 bus, Uint8 device, Uint8 function, Uint8 offset);
Uint8  __pci_config_read_byte(Uint8 bus, Uint8 device, Uint8 function, Uint8 offset);

void __pci_config_write_long(Uint8 bus, Uint8 device, Uint8 function, Uint8 offset, Uint32 val);
void __pci_config_write_short(Uint8 bus, Uint8 device, Uint8 function, Uint8 offset, Uint16 val);
void __pci_config_write_byte(Uint8 bus, Uint8 device, Uint8 function, Uint8 offset, Uint8 val);

void __pci_dump_all_devices(void);
void __pci_dump_device(Uint8 bus, Uint8 slot, Uint8 function);

#endif
