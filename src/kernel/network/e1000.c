#include "e1000.h"
#include "../pci/pci.h"
#include "../lib/klib.h"
#include "../serial.h"

#define PCI_NETWORK_CLASS 0x02
#define PCI_ETHERNET 0x00
#define PCI_ETHERNET_PROG_IF 0x00

void __net_init(void)
{
	pci_config_t* pci_network = __pci_find_by_class(PCI_NETWORK_CLASS, PCI_ETHERNET, PCI_ETHERNET_PROG_IF);

	if (pci_network == NULL)
	{
		_kpanic("Network", "Couldn't find network device!\n", 0);
	}

	serial_printf("Network device found!\n");
}
