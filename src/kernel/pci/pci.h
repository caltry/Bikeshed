#ifndef __PCI_H__
#define __PCI_H__

#include "headers.h"

#define PCI_CONFIG_SPACE_PORT 0xCF8
#define PCI_CONFIG_DATA_PORT  0xCFC
#define ENABLE_PCI_CONFIG_SPACE 0x80000000

#define PCI_VENDOR_ID 0x0
#define PCI_DEVICE_ID 0x2
#define PCI_CLASS 0x8
#define PCI_CACHE_LINE_SIZE 0xC
#define PCI_LATENCY_TIMER 0xD
#define PCI_HEADER_TYPE 0xE
#define PCI_BIST 0xF
#define PCI_CLASS_REVISION 0x08 /* Highest 24-bits are class, low are revision */

// The format of CONFIG_ADDRESS is the following:
// bus << 16  |  device << 11  |  function <<  8  |  offset

// 31 	        30 - 24 	23 - 16 	15 - 11 	    10 - 8 	            7 - 2 	            1 - 0 
// Enable Bit 	Reserved 	Bus Number 	Device Number 	Function Number 	Register Number 	00 

// Useful wiki
// http://en.wikipedia.org/wiki/Input/Output_Base_Address

typedef struct PCIHeaderType0
{
	Uint32 bar_address_0;
	Uint32 bar_address_1;
	Uint32 bar_address_2;
	Uint32 bar_address_3;
	Uint32 bar_address_4;
	Uint32 bar_address_5;

	Uint32 cardbus_cis_pointer;
	Uint16 subsystem_vendor_id;
	Uint16 subsystem_id;

	Uint32 expansion_rom_address;
	Uint8 capabilities_pointer;
	Uint8 reserved[7];
	Uint8 interrupt_line;
	Uint8 interrupt_pin;
	Uint8 min_grant;
	Uint8 max_latency;
} header_type_0;

typedef struct PCIHeaderType1
{
	Uint32 bar_address_0;
	Uint32 bar_address_1;

	Uint8 primary_bus_number;
	Uint8 secondary_bus_number;
	Uint8 subordinate_bus_number;
	Uint8 secondary_latency_timer;

	Uint8 io_base;
	Uint8 io_limit;
	Uint16 secondary_status;

	Uint16 memory_base;
	Uint16 memory_limit;

	Uint16 prefetchable_memory_base;
	Uint16 prefetchable_memory_limit;

	Uint32 prefetchable_base_upper_32_bits;
	Uint32 prefetchable_memory_limit_upper_32_bits;

	Uint16 io_base_upper_16_bits;
	Uint16 io_limit_upper_16_bits;

	Uint8 capability_pointer;
	Uint8 reserved[3];

	Uint32 expansion_rom_base_address;
	Uint8 interrupt_line;
	Uint8 interrupt_pin;
	Uint16 bridge_control;
} header_type_1;

typedef struct PCIHeaderType2
{
	Uint32 cardbus_socket_base_address;

	Uint8 offset_of_capabilities_list;
	Uint8 reserved;
	Uint16 secondary_status;

	Uint8 pci_bus_number;
	Uint8 card_bus_number;
	Uint8 subordinate_bus_number;
	Uint8 cardbus_latency_timer;

	Uint32 memory_base_address_0;
	Uint32 memory_limit_0;

	Uint32 memory_base_address_1;
	Uint32 memory_limit_1;

	Uint32 io_base_address_0;
	Uint32 io_limit_0;

	Uint32 io_base_address_1;
	Uint32 io_limit_1;

	Uint8 interrupt_line;
	Uint8 interrupt_pin;

	Uint16 bridge_control;

	Uint16 subsystem_device_id;
	Uint16 subsystem_vendor_id;

	Uint32 pci_legacy_mode_base_address;
} header_type_2;

typedef struct PCIConfig
{
	Uint8 bus;
	Uint8 slot;
	Uint8 function;

	Uint16 vendor_id;
	Uint16 device_id;
	Uint8 base_class;
	Uint8 sub_class;
	Uint8 programming_if;
	Uint8 revision;
	Uint8 BIST;
	Uint8 header_type;
	Uint8 latency_timer;
	Uint8 cache_line_size;

	union 
	{
		header_type_0 type_0;	
		header_type_1 type_1;
		header_type_2 type_2;
	};
} pci_config_t;

/* Initialize the pci devices
 */
void __pci_init(void);

Uint32 __pci_config_read_long(Uint8 bus, Uint8 device, Uint8 function, Uint8 offset);
Uint16 __pci_config_read_short(Uint8 bus, Uint8 device, Uint8 function, Uint8 offset);
Uint8  __pci_config_read_byte(Uint8 bus, Uint8 device, Uint8 function, Uint8 offset);

void __pci_config_write_long(Uint8 bus, Uint8 device, Uint8 function, Uint8 offset, Uint32 val);
void __pci_config_write_short(Uint8 bus, Uint8 device, Uint8 function, Uint8 offset, Uint16 val);
void __pci_config_write_byte(Uint8 bus, Uint8 device, Uint8 function, Uint8 offset, Uint8 val);

void __pci_scan_devices(void);
void __pci_dump_all_devices(void);
void __pci_dump_device(Uint8 bus, Uint8 slot, Uint8 function);

#endif
