#include "pci.h"
#include "../serial.h"
#include "../../boot/startup.h"
#include "../memory/kmalloc.h"
#include "../data_structures/linkedlist.h"
#include "../lib/klib.h"

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

// TODO REMOVE, this was a test function
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

				// Mask off the special MF flag
				switch ((pci_config->header_type & 0x7))
				{
					case 0:
						{
							pci_config->type_0.bar_address_0 = __pci_config_read_long(bus, slot, func, 0x10);
							pci_config->type_0.bar_address_1 = __pci_config_read_long(bus, slot, func, 0x14);
							pci_config->type_0.bar_address_2 = __pci_config_read_long(bus, slot, func, 0x18);
							pci_config->type_0.bar_address_3 = __pci_config_read_long(bus, slot, func, 0x1C);
							pci_config->type_0.bar_address_4 = __pci_config_read_long(bus, slot, func, 0x20);
							pci_config->type_0.bar_address_5 = __pci_config_read_long(bus, slot, func, 0x24);
							pci_config->type_0.cardbus_cis_pointer = __pci_config_read_long(bus, slot, func, 0x28);
							pci_config->type_0.subsystem_vendor_id = __pci_config_read_short(bus, slot, func, 0x2C);
							pci_config->type_0.subsystem_id = __pci_config_read_short(bus, slot, func, 0x2E);
							pci_config->type_0.expansion_rom_address = __pci_config_read_long(bus, slot, func, 0x30);
							pci_config->type_0.capabilities_pointer = __pci_config_read_byte(bus, slot, func, 0x34);
							pci_config->type_0.interrupt_line = __pci_config_read_byte(bus, slot, func, 0x3C);
							pci_config->type_0.interrupt_pin = __pci_config_read_byte(bus, slot, func, 0x3D);
							pci_config->type_0.min_grant = __pci_config_read_byte(bus, slot, func, 0x3E);
							pci_config->type_0.max_latency = __pci_config_read_byte(bus, slot, func, 0x3F);
						}
						break;
					case 1:
						{
							pci_config->type_1.bar_address_0 = __pci_config_read_long(bus, slot, func, 0x10);
							pci_config->type_1.bar_address_1 = __pci_config_read_long(bus, slot, func, 0x14);
							pci_config->type_1.primary_bus_number = __pci_config_read_byte(bus, slot, func, 0x18);
							pci_config->type_1.secondary_bus_number = __pci_config_read_byte(bus, slot, func, 0x19);
							pci_config->type_1.subordinate_bus_number = __pci_config_read_byte(bus, slot, func, 0x1A);
							pci_config->type_1.secondary_latency_timer = __pci_config_read_byte(bus, slot, func, 0x1B);
							pci_config->type_1.io_base = __pci_config_read_byte(bus, slot, func, 0x1C);
							pci_config->type_1.io_limit = __pci_config_read_byte(bus, slot, func, 0x1D);
							pci_config->type_1.secondary_status = __pci_config_read_short(bus, slot, func, 0x1E);
							pci_config->type_1.memory_base = __pci_config_read_short(bus, slot, func, 0x20);
							pci_config->type_1.memory_limit = __pci_config_read_short(bus, slot, func, 0x22);
							pci_config->type_1.prefetchable_memory_base = __pci_config_read_short(bus, slot, func, 0x24);
							pci_config->type_1.prefetchable_memory_limit = __pci_config_read_short(bus, slot, func, 0x26);
							pci_config->type_1.prefetchable_base_upper_32_bits = __pci_config_read_long(bus, slot, func, 0x28);
							pci_config->type_1.prefetchable_memory_limit_upper_32_bits = __pci_config_read_long(bus, slot, func, 0x2C);
							pci_config->type_1.io_base_upper_16_bits = __pci_config_read_short(bus, slot, func, 0x30);
							pci_config->type_1.io_limit_upper_16_bits = __pci_config_read_short(bus, slot, func, 0x32);
							pci_config->type_1.capability_pointer = __pci_config_read_byte(bus, slot, func, 0x34);
							pci_config->type_1.expansion_rom_base_address = __pci_config_read_long(bus, slot, func, 0x38);
							pci_config->type_1.interrupt_line = __pci_config_read_byte(bus, slot, func, 0x3C);
							pci_config->type_1.interrupt_pin = __pci_config_read_byte(bus, slot, func, 0x3D);
							pci_config->type_1.bridge_control = __pci_config_read_short(bus, slot, func, 0x3E);
						}
						break;
					case 2:
						{
							pci_config->type_2.cardbus_socket_base_address = __pci_config_read_long(bus, slot, func, 0x10);
							pci_config->type_2.offset_of_capabilities_list = __pci_config_read_byte(bus, slot, func, 0x14);
							pci_config->type_2.secondary_status = __pci_config_read_short(bus, slot, func, 0x16);
							pci_config->type_2.pci_bus_number = __pci_config_read_byte(bus, slot, func, 0x18);
							pci_config->type_2.card_bus_number = __pci_config_read_byte(bus, slot, func, 0x19);
							pci_config->type_2.subordinate_bus_number = __pci_config_read_byte(bus, slot, func, 0x1A);
							pci_config->type_2.cardbus_latency_timer = __pci_config_read_byte(bus, slot, func, 0x1B);
							pci_config->type_2.memory_base_address_0 = __pci_config_read_long(bus, slot, func, 0x1C);
							pci_config->type_2.memory_limit_0 = __pci_config_read_long(bus, slot, func, 0x20);
							pci_config->type_2.memory_base_address_1 = __pci_config_read_long(bus, slot, func, 0x24);
							pci_config->type_2.memory_limit_1 = __pci_config_read_long(bus, slot, func, 0x28);
							pci_config->type_2.io_base_address_0 = __pci_config_read_long(bus, slot, func, 0x2C);
							pci_config->type_2.io_limit_0 = __pci_config_read_long(bus, slot, func, 0x30);
							pci_config->type_2.io_base_address_1 = __pci_config_read_long(bus, slot, func, 0x34);
							pci_config->type_2.io_limit_1 = __pci_config_read_long(bus, slot, func, 0x38);
							pci_config->type_2.interrupt_line = __pci_config_read_byte(bus, slot, func, 0x3C);
							pci_config->type_2.interrupt_pin = __pci_config_read_byte(bus, slot, func, 0x3D);
							pci_config->type_2.bridge_control = __pci_config_read_short(bus, slot, func, 0x3E);
							pci_config->type_2.subsystem_device_id = __pci_config_read_short(bus, slot, func, 0x40);
							pci_config->type_2.subsystem_vendor_id = __pci_config_read_short(bus, slot, func, 0x42);
							pci_config->type_2.pci_legacy_mode_base_address = __pci_config_read_long(bus, slot, func, 0x44);
						}
						break;
					default:
						// Uh oh?
						_kpanic("PCI", "Device found with bad header type\n", 0);
						break;
				}

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
		serial_printf("BIST         : %02xh\n", config->BIST);

		switch ((config->header_type & 0x7))
		{
			case 0:
				{
					serial_printf("\nHeader type: 0\n");	
					serial_printf("Bar address 0: %x\n", config->type_0.bar_address_0);
					serial_printf("Bar address 1: %x\n", config->type_0.bar_address_1);
					serial_printf("Bar address 2: %x\n", config->type_0.bar_address_2);
					serial_printf("Bar address 3: %x\n", config->type_0.bar_address_3);
					serial_printf("Bar address 4: %x\n", config->type_0.bar_address_4);
					serial_printf("Bar address 5: %x\n", config->type_0.bar_address_5);
					serial_printf("Carbus pointer: %x\n", config->type_0.cardbus_cis_pointer);
					serial_printf("Subsystem Vendor ID: %x\n", config->type_0.subsystem_vendor_id);
					serial_printf("Subsystem ID: %x\n", config->type_0.subsystem_id);
					serial_printf("Expansion ROM addr: %x\n", config->type_0.expansion_rom_address);
					serial_printf("Capabilities Pointer: %x\n", config->type_0.capabilities_pointer);
					serial_printf("Interrupt line: %x\n", config->type_0.interrupt_line);
					serial_printf("Interrupt pin: %x\n", config->type_0.interrupt_pin);
					serial_printf("Min grant: %x\n", config->type_0.min_grant);
					serial_printf("Max latency: %x\n", config->type_0.max_latency);
				}
				break;
			case 1:
				{
					serial_printf("\nHeader type: 1\n");
					serial_printf("Bar address 0: %x\n", config->type_1.bar_address_0);
					serial_printf("Bar address 1: %x\n", config->type_1.bar_address_1);
					serial_printf("Primary bus number: %x\n", config->type_1.primary_bus_number);
					serial_printf("Secondary bus number: %x\n", config->type_1.secondary_bus_number);
					serial_printf("Subordinate bus number: %x\n", config->type_1.subordinate_bus_number);
					serial_printf("Secondary latency timer: %x\n", config->type_1.secondary_latency_timer);
					serial_printf("IO base: %x\n", config->type_1.io_base);
					serial_printf("IO limit: %x\n", config->type_1.io_limit);
					serial_printf("Secondary status: %x\n", config->type_1.secondary_status);
					serial_printf("Memory base: %x\n", config->type_1.memory_base);
					serial_printf("Memory limit: %x\n", config->type_1.memory_limit);
					serial_printf("Prefetch memory base: %x\n", config->type_1.prefetchable_memory_base);
					serial_printf("Prefetch memory limit: %x\n", config->type_1.prefetchable_memory_limit);
					serial_printf("Prefetch base upper 32-bits: %x\n", config->type_1.prefetchable_base_upper_32_bits);
					serial_printf("Prefetch limit upper 32-bits: %x\n", config->type_1.prefetchable_memory_limit_upper_32_bits);
					serial_printf("IO base upper 16-bits: %x\n", config->type_1.io_base_upper_16_bits);
					serial_printf("IO limit upper 16-bits: %x\n", config->type_1.io_limit_upper_16_bits);
					serial_printf("Capability pointer: %x\n", config->type_1.capability_pointer);
					serial_printf("Expansion ROM base address: %x\n", config->type_1.expansion_rom_base_address);
					serial_printf("Interrupt line: %x\n", config->type_1.interrupt_line);
					serial_printf("Interrupt pin: %x\n", config->type_1.interrupt_pin);
					serial_printf("Bridge control: %x\n", config->type_1.bridge_control);
				}
				break;
			case 2:
				{
					serial_printf("Cardbus socket base address: %x\n", config->type_2.cardbus_socket_base_address);
					serial_printf("Offset of capabilities: %x\n", config->type_2.offset_of_capabilities_list);
					serial_printf("Secondary status: %x\n", config->type_2.secondary_status);
					serial_printf("PCI bus number: %x\n", config->type_2.pci_bus_number);
					serial_printf("Cardbus bus number: %x\n", config->type_2.card_bus_number);
					serial_printf("Subordinate bus number: %x\n", config->type_2.subordinate_bus_number);
					serial_printf("Cardbus latency timer: %x\n", config->type_2.cardbus_latency_timer);
					serial_printf("Memory base address 0: %x\n", config->type_2.memory_base_address_0);
					serial_printf("Memory limit 0: %x\n", config->type_2.memory_limit_0);
					serial_printf("Memory base address 1: %x\n", config->type_2.memory_base_address_1);
					serial_printf("Memory limit 1: %x\n", config->type_2.memory_limit_1);
					serial_printf("IO base address 0: %x\n", config->type_2.io_base_address_0);
					serial_printf("IO limit 0: %x\n", config->type_2.io_limit_0);
					serial_printf("IO base address 1: %x\n", config->type_2.io_base_address_1);
					serial_printf("IO limit 1: %x\n", config->type_2.io_limit_1);
					serial_printf("Interrupt line: %x\n", config->type_2.interrupt_line);
					serial_printf("Interrupt pin: %x\n", config->type_2.interrupt_pin);
					serial_printf("Bridge control: %x\n", config->type_2.bridge_control);
					serial_printf("Subsystem device ID: %x\n", config->type_2.subsystem_device_id);
					serial_printf("Subsystem vendor ID: %x\n", config->type_2.subsystem_vendor_id);
					serial_printf("16-bit PC card legacy base address: %x\n", config->type_2.pci_legacy_mode_base_address);
				}
				break;
		}

		serial_printf("\n");
		node = list_next(node);
	}
}

pci_config_t* __pci_find_by_class(Uint8 base_class, Uint8 sub_class, Uint8 prog_if)
{
	list_element_t* node = list_head(lst_pci_devices);

	while (node != NULL)
	{
		pci_config_t* pci_info = (pci_config_t *)list_data(node);
		if (pci_info->base_class == base_class && 
				pci_info->sub_class == sub_class &&
				pci_info->programming_if == prog_if)
		{
			return pci_info;
		}

		node = list_next(node);
	}

	return NULL;
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

