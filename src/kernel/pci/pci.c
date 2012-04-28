#include "pci.h"
#include "../serial.h"
#include "../../boot/startup.h"

Uint32 __pci_config_read_long(Uint8 bus, Uint8 device, Uint8 function, Uint8 offset)
{
	const Uint32 val = ENABLE_PCI_CONFIG_SPACE | bus << 16 | device << 11 | function << 8 | offset;
	__outl(val, PCI_CONFIG_SPACE_PORT);
	Uint32 output = __inl(PCI_CONFIG_DATA_PORT);

	return output;
}

Uint16 __pci_config_read_short(Uint8 bus, Uint8 device, Uint8 function, Uint8 offset)
{
	const Uint32 val = ENABLE_PCI_CONFIG_SPACE | bus << 16 | device << 11 | function << 8 | offset;
	__outl(val, PCI_CONFIG_SPACE_PORT);
	Uint16 output = __inw(PCI_CONFIG_DATA_PORT + (offset & 2));
	
	return output;
}

Uint8 __pci_config_read_byte(Uint8 bus, Uint8 device, Uint8 function, Uint8 offset)
{
	const Uint32 val = ENABLE_PCI_CONFIG_SPACE | bus << 16 | device << 11 | function << 8 | offset;
	__outl(val, PCI_CONFIG_SPACE_PORT);
	Uint8 output = __inb(PCI_CONFIG_DATA_PORT + (offset & 3));
	
	return output;
}

void __pci_config_write_long(Uint8 bus, Uint8 device, Uint8 function, Uint8 offset, Uint32 val)
{
	const Uint32 pci = ENABLE_PCI_CONFIG_SPACE | bus << 16 | device << 11 | function << 8 | offset;
	__outl(pci, PCI_CONFIG_SPACE_PORT);	
	__outl(val, PCI_CONFIG_DATA_PORT);
}

void __pci_config_write_short(Uint8 bus, Uint8 device, Uint8 function, Uint8 offset, Uint16 val)
{
	const Uint32 pci = ENABLE_PCI_CONFIG_SPACE | bus << 16 | device << 11 | function << 8 | offset;
	__outl(pci, PCI_CONFIG_SPACE_PORT);
	__outw(val, PCI_CONFIG_DATA_PORT + (offset & 2));
}

void __pci_config_write_byte(Uint8 bus, Uint8 device, Uint8 function, Uint8 offset, Uint8 val)
{
	const Uint32 pci = ENABLE_PCI_CONFIG_SPACE | bus << 16 | device << 11 | function << 8 | offset;
	__outl(pci, PCI_CONFIG_SPACE_PORT);	
	__outb(val, PCI_CONFIG_DATA_PORT + (offset & 3));
}

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

					if (!(type & 0x80)) /* TODO What does this mean? */
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
		if (!(i & 0x0f)) /* TODO What does this mean? */
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

void __pci_init()
{
	
}

