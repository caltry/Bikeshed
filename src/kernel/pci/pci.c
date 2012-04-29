#include "pci.h"
#include "../serial.h"
#include "../../boot/startup.h"

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

void __pci_dump_all_devices()
{
	for (Uint32 bus = 0; bus < 256; ++bus)
	{
		for (Uint32 slot = 0; slot < 32; ++slot)
		{
			for (Uint32 func = 0; func < 8; ++func)
			{
				//Uint32 vendor = pciConfigReadWord(bus, 0, 0, 0);//__pci_config_read_short(bus, 0, 0, PCI_VENDOR_ID);
				Uint32 vendor = __pci_config_read_short(bus, slot, 0, 0);
				if (vendor == 0xFFFF) 
				{
					continue; 
				}

				Uint32 device_id = pciConfigReadWord(bus, slot, func, 0x2);//__pci_config_read_short(bus, 0, 0, 0x2);

				if (device_id == 0xFFFF)
				{
					continue;
				}

				// For class codes: (3 bytes) Class, Sub class, Prog IF

				serial_printf("pci 0000:%d:%d.%d\n", bus, slot, func);
				serial_printf("vendor: %x\n", vendor);
				serial_printf("device: %x\n", device_id);
				device_id = __pci_config_read_short(bus, slot, func, 0x2);	
				serial_printf("device: %x\n", device_id);
				//serial_printf("Vendor+device: %08x\n", __pci_config_read_long(bus, slot, 0, 0));
				//serial_printf("byte test: %x\n", __pci_config_read_byte(bus, slot, 0, 0));

				//Uint32 class = __pci_config_read_long(bus, 0, 0, 0x8);
				Uint32 class = 
					pciConfigReadWord(bus, slot, func, 0xa) << 16 |
					pciConfigReadWord(bus, slot, func, 0x8);
				serial_printf("class: %08x\n", class);
				class = __pci_config_read_long(bus, slot, func, 0x8);
				serial_printf("class: %08x\n", class);

				Uint32 subsystem_vendor = __pci_config_read_short(bus, slot, func, 0x2c);
				Uint32 subsystem_id = __pci_config_read_short(bus, slot, func, 0x2e);
				serial_printf("subsystem vendor: %x\n", subsystem_vendor);
				serial_printf("subsystem id: %x\n", subsystem_id);

				__pci_dump_device(bus, slot, func);
				serial_printf("\n");
			}
		}
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
	
}

