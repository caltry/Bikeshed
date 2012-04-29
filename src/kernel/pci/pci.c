#include "pci.h"
#include "../serial.h"
#include "../../boot/startup.h"
#include "../memory/kmalloc.h"
#include "../data_structures/linkedlist.h"

linked_list_t* lst_pci_devices;

Uint32 __pci_config_read_long(Uint8 bus, Uint8 device, Uint8 function, Uint8 offset)
{
	const Uint32 address = ENABLE_PCI_CONFIG_SPACE | bus << 16 | device << 11 | 
						function << 8 | (offset & 0xFC);
	__outl(PCI_CONFIG_SPACE_PORT, address);
	Uint32 output = __inl(PCI_CONFIG_DATA_PORT);

	return output;
}

Uint16 __pci_config_read_short(Uint8 bus, Uint8 device, Uint8 function, Uint8 offset)
{
	const Uint32 address = ENABLE_PCI_CONFIG_SPACE | bus << 16 | device << 11 | 
					 function << 8 | (offset & 0xFC);
	__outl(PCI_CONFIG_SPACE_PORT, address);
	Uint16 output = (__inl(PCI_CONFIG_DATA_PORT) >> ((offset & 2) * 8)) & 0xffff;
	
	return output;
}

Uint8 __pci_config_read_byte(Uint8 bus, Uint8 device, Uint8 function, Uint8 offset)
{
	const Uint32 address = ENABLE_PCI_CONFIG_SPACE | bus << 16 | device << 11 | function << 8 | (offset & 0xFC);
	__outl(PCI_CONFIG_SPACE_PORT, address);
	Uint8 output = (__inl(PCI_CONFIG_DATA_PORT) >> ((offset & 2) * 8)) & 0xFF;
	
	return output;
}

void __pci_config_write_long(Uint8 bus, Uint8 device, Uint8 function, Uint8 offset, Uint32 val)
{
	const Uint32 pci = ENABLE_PCI_CONFIG_SPACE | bus << 16 | device << 11 | function << 8 | (offset & 0xFC);
	__outl(PCI_CONFIG_SPACE_PORT, pci);	
	__outl(PCI_CONFIG_DATA_PORT, val);
}

void __pci_config_write_short(Uint8 bus, Uint8 device, Uint8 function, Uint8 offset, Uint16 val)
{
	const Uint32 pci = ENABLE_PCI_CONFIG_SPACE | bus << 16 | device << 11 | function << 8 | (offset & 0xFC);
	__outl(PCI_CONFIG_SPACE_PORT, pci);
	__outw(PCI_CONFIG_DATA_PORT, val);// + (offset & 2));
}

void __pci_config_write_byte(Uint8 bus, Uint8 device, Uint8 function, Uint8 offset, Uint8 val)
{
	const Uint32 pci = ENABLE_PCI_CONFIG_SPACE | bus << 16 | device << 11 | function << 8 | (offset & 0xFC);
	__outl(PCI_CONFIG_SPACE_PORT, pci);	
	__outb(PCI_CONFIG_DATA_PORT, val);// + (offset & 3));
}
/*
void __pci_dump_all_devices()
{
	Uint16 bus, slot, func = 0;

	for (bus = 0; bus < 256; ++bus)
	{
		for (slot = 0; slot < 32; ++slot)
		{
			for (func = 0; func < 8; ++func)
			{
				Uint32 class;
				Uint8 type;

				class = __pci_config_read_long(bus, slot, func, PCI_CLASS_REVISION);

				if (class == 0xFFFFFFFF)
					continue;
				
				__pci_dump_device(bus, slot, func);

				if (func == 0)
				{
					type = __pci_config_read_byte(bus, slot, func, PCI_HEADER_TYPE);

					if (!(type & 0x80)) // TODO What does this mean? 
						break;
				}
			}
		}
	}
}

void __pci_dump_device(Uint8 bus, Uint8 slot, Uint8 func)
{
	serial_printf("pci 0000:%d:%d.%d", bus, slot, func);

	for (Int32 i = 0; i < 256; i += 4)
	{
		if (!(i & 0x0f)) // TODO What does this mean? 
		{
			serial_printf("\n %d:", i);
		}

		Uint32 val = __pci_config_read_long(bus, slot, func, i);
		for (Int32 j = 0; j < 4; ++j)
		{
			serial_printf(" %d", val & 0xFF);
			val >>= 8;
		}
	}
	serial_string("\n");
}
*/

unsigned short pciConfigReadWord (unsigned short bus, unsigned short slot,
		unsigned short func, unsigned short offset)
{
	unsigned long address;
	unsigned long lbus = (unsigned long)bus;
	unsigned long lslot = (unsigned long)slot;
	unsigned long lfunc = (unsigned long)func;
	unsigned short tmp = 0;

	/* create configuration address as per Figure 1 */
	address = (unsigned long)((lbus << 16) | (lslot << 11) |
			(lfunc << 8) | (offset & 0xfc) | ((Uint32)0x80000000));

	/* write out the address */
	__outl (0xCF8, address);
	/* read in the data */
	/* (offset & 2) * 8) = 0 will choose the first word of the 32 bits register */
	tmp = (unsigned short)((__inl (0xCFC) >> ((offset & 2) * 8)) & 0xffff);
	return (tmp);
}

void __pci_scan_devices()
{
	for (Uint32 bus = 0; bus < 256; ++bus)
	{
		for (Uint32 slot = 0; slot < 32; ++slot)
		{
			for (Uint32 func = 0; func < 8; ++func)
			{
				Uint32 vendor_id = __pci_config_read_short(bus, slot, 0, PCI_VENDOR_ID);
				if (vendor_id == 0xFFFF) 
				{
					continue; 
				}

				Uint32 device_id = __pci_config_read_short(bus, slot, func, PCI_DEVICE_ID);
				if (device_id == 0xFFFF)
				{
					continue;
				}

				// Okay this is a valid pci device!
				pci_config_t* pci_config = (pci_config_t *)__kmalloc(sizeof(pci_config_t));

				pci_config->bus  = bus;
				pci_config->slot = slot;
				pci_config->function = func;

				pci_config->vendor_id = vendor_id;
				pci_config->device_id = device_id;

				Uint32 class = __pci_config_read_long(bus, slot, func, PCI_CLASS);
				pci_config->base_class     = (class >> 24) & 0xFF;
				pci_config->sub_class      = (class >> 16) & 0xFF;
				pci_config->programming_if = (class >> 8) & 0xFF;
				pci_config->revision       = class & 0xFF;

				pci_config->cache_line_size = __pci_config_read_byte(bus, slot, func, PCI_CACHE_LINE_SIZE);
				pci_config->latency_timer = __pci_config_read_byte(bus, slot, func, PCI_LATENCY_TIMER);
				pci_config->header_type = __pci_config_read_byte(bus, slot, func, PCI_HEADER_TYPE);
				pci_config->BIST = __pci_config_read_byte(bus, slot, func, PCI_BIST);

				// Add this device to our devices list
				list_insert_next(lst_pci_devices, NULL, pci_config);
			}
		}
	}
}

void __pci_dump_all_devices()
{
	list_element_t* node = list_head(lst_pci_devices);			

	serial_printf("\nPCI devices list:\n");

	while (node != NULL)
	{
		pci_config_t* config = (pci_config_t *)list_data(node);

		serial_printf("Bus: %d, Slot: %d, Function: %d\n", config->bus, config->slot, config->function);
		serial_printf("Vendor ID    : %04xh\n", config->vendor_id);
		serial_printf("Device ID    : %04xh\n", config->device_id);
		serial_printf("Base Class   : %02xh\n", config->base_class);
		serial_printf("Sub  Class   : %02xh\n", config->sub_class);
		serial_printf("Prog IF      : %02xh\n", config->programming_if);
		serial_printf("Revision     : %02xh\n", config->revision);
		serial_printf("Cache Size   : %02xh\n", config->cache_line_size);
		serial_printf("Latency Timer: %02xh\n", config->latency_timer);
		serial_printf("Header Type  : %02xh\n", config->header_type);
		serial_printf("BIST         : %02xh\n\n", config->BIST);

		node = list_next(node);
	}
}

void __pci_dump_device(Uint8 bus, Uint8 slot, Uint8 func)
{
	//serial_printf("pci 0000:%d:%d.%d", bus, slot, func);
	UNUSED(bus);
	UNUSED(slot);
	UNUSED(func);
}

void __pci_init()
{
	list_init(lst_pci_devices, __kfree);	
	__pci_scan_devices();
}

