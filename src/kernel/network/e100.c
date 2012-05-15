#include "e100.h"
#include "defs.h"
#include "pci/pci.h"
#include "lib/klib.h"
#include "serial.h"
#include "support.h"

/* The PCI configuration space for the e100 series of network controllers
 * looks like the following:
 * .----------------------------------------------------.
 * |Byte offset |  byte 3 |  byte 2 |  byte 1 |  byte 0 |
 * |------------+-------------------+-------------------|
 * |      0     | Device ID         | Vendor ID         |        
 * |------------+-------------------+-------------------|
 * |      4     | Status Register   | Command Register  |
 * |------------+-------------------+---------+---------|
 * |      8     | Class Code (0x200000)       | Revision|
 * |------------+-----------------------------+---------|
 * |      C     | BIST    |Head Type|Lat Timer|Chc Line |
 * |------------+---------------------------------------|
 * |     10     |CSR Memory Mapped Base Address Register|
 * |------------+---------------------------------------|
 * |     14     |CSR I/O Mapped Base Address Register   |
 * |------------+---------------------------------------|
 * |     18     |Flash mem mapped base address register |
 * |----------------------------------------------------|
 * |                     .......                        |
 * '----------------------------------------------------'
 *
 * Command Register at offset 4
 * .-------------------------.
 * | 15    10 | 9          0 |
 * |----------+--------------|
 * | Reserved | Command Bits |      
 * '-------------------------'
 * 
 * .------------------------------------------------.
 * | Bits  | Init Val | Description                 |
 * |-------+----------+-----------------------------|
 * | 15:10 |    0     | Reserved                    |
 * |-------+----------+-----------------------------|
 * | 9     |    0     | Fast back-to-back enable    |
 * |-------+----------+-----------------------------|
 * | 8     |    x     | SERR# enable                |
 * |-------+----------+-----------------------------|
 * | 7     |    0     | Wait cycle enable           |
 * |-------+----------+-----------------------------|
 * | 6     |    x     | Parity error response       |
 * |-------+----------+-----------------------------|
 * | 5     |    0     | Palette snoop enable        |
 * |-------+----------+-----------------------------|
 * | 4     |    x     | Memory write and invalidate |
 * |-------+----------+-----------------------------|
 * | 3     |    0     | Special cycle monitoring    |
 * |-------+----------+-----------------------------|
 * | 2     |    x     | Mastering enable            |
 * |-------+----------+-----------------------------|
 * | 1     |    x     | Memory access enable        |
 * |-------+----------+-----------------------------|
 * | 0     |    x     | I/O access enable           |
 * '------------------------------------------------'
 *
 * Status Register at offset 6
 * .------------------------.
 * | 15        9 | 3      0 |
 * |------------------------|
 * | Status Bits | Reserved |
 * '------------------------'
 *
 * .-----------------------------------------------------------------.
 * | Bits | Init Val | Description                                   |
 * |------+----------+-----------------------------------------------|
 * | 15   |    x     | Detect parity error                           |
 * |------+----------+-----------------------------------------------|
 * | 14   |    x     | Signaled system error                         |
 * |------+----------+-----------------------------------------------|
 * | 13   |    x     | Received master abort                         |
 * |------+----------+-----------------------------------------------|
 * | 12   |    x     | Received target abort                         |
 * |------+----------+-----------------------------------------------|
 * | 11   |    0     | Signaled target abort                         |
 * |------+----------+-----------------------------------------------|
 * | 10:9 |    01    | DEVSEL timing (indicated minimum timing)      |
 * |------+----------+-----------------------------------------------|
 * | 8    |    x     | Data parity reported                          |
 * |------+----------+-----------------------------------------------|
 * | 7    |    1     | Fast back-to-back capable                     |
 * |------+----------+-----------------------------------------------|
 * | 6    |    0     | UDF supported                                 |
 * |------+----------+-----------------------------------------------|
 * | 5    |    0     | 66 MHz capable                                |
 * |------+----------+-----------------------------------------------|
 * | 4    |  1 or 0  | 1 for 82559/8 and 0 82557 - Capabilities list |
 * |------+----------+-----------------------------------------------|
 * | 3:0  |    0     | Reserved                                      |
 * '-----------------------------------------------------------------'
 *
 * Initialize Cache Line Size (offset C)
 *
 *
 * Port register commands
 * .--------------------------------------------------------------.
 * | Bits 31:16 (Upper Word) | Bits 15:0 (Lower Word) | Offset    |
 * |-------------------------+------------------------+-----------|
 * |    SCB Command Word     |    SCB Status Word     | Base + 0h |
 * |-------------------------+------------------------+-----------|
 * |                 SCB General Pointer              | Base + 4h |
 * |--------------------------------------------------+-----------|
 * |                       PORT                       | Base + 8h |
 * '--------------------------------------------------------------'
 *
 * Port selection function
 * Function       - Pointer Field (Bits 31:4) - Opcode (Bits 3:0)
 * Software Reset - Don't care                - 0000
 * Self-test      - Self-test results pointer - 0001
 *                - (16 byte alignment)
 * Selective Reset- Don't care                - 0010
 * Dump           - Dump area pointer
 *                - (16 byte alignment)     
 * Dump Wake-up   - Dump area pointer         - 0011
 *                - (16 byte alignment)
 */

static void __isr_nic_card(int vector, int code)
{
	UNUSED(vector);
	UNUSED(code);

	serial_printf("\t\tNIC interrupt!\n");
}

#define PCI_NETWORK_CLASS 0x02
#define PCI_ETHERNET 0x00
#define PCI_ETHERNET_PROG_IF 0x00

/* PCI commands */
#define PCI_MR 0x6 // TxCB "S" bit read
#define PCI_MW 0x7 // CB and RFD write statistics counter, or dump data buffer to memory.
// Write received packet data into receive buffers
#define PCI_MRM 0xC // Reading transmit data buffers
#define PCI_MRL 0xE // CB, TBD, and RFD
#define PCI_MWI 0xF // Writing received packet data into receive buffers

#define CACHE_LINE_SIZE 0xC

#define SCB_STATUS_WORD  0x0 /* For use with 16-bit pointers */
#define SCB_COMMAND_WORD 0x1 /* Offset 0x2, but 0x1 with a 16-bit pointer */
#define SCB_GENERAL_PTR  0x1 /* Offset of 0x4, but 0x1 with 32-bit pointer */
#define PORT 0x2 /* Offset of 0x8, but 0x2 with a 32-bit pointer */

static const pci_config_t* pci_e100;
static Uint8*  base_address_8; 
static Uint16* base_address_16;
static Uint32* base_address_32;

/* Interrupt types */
#define CX_TNO 0x8000 /* Asserted when the CU has finished executing a command */
#define FR     0x4000 /* The RU has finished receiving a frame */
#define CNA    0x2000 /* CU has transitioned active -> idle */
#define RNR    0x1000 /* RU left the ready state */
#define MDI    0x0800 /* An MDI read or write cycle has completed */
#define SWI    0x0400 /* Software generated interrupt, used to re-enter the ISR */
#define FCP    0x0100 /* Flow Control Pause, not used on the 82557 */
/****/

/* CUS types */
#define CUS_IDLE       0x0
#define CUS_SUSPENDED  0x1
#define CUS_LPQ_ACTIVE 0x2
#define CUS_HQP_ACTIVE 0x3
/****/

/* RUS types */
#define RUS_IDLE         0x00
#define RUS_SUSPENDED    0x01
#define RUS_NO_RESOURCES 0x02
#define RUS_READY        0x10
/****/

/* May need a state machine to keep track of what commands have been issued
 * the documentation says you cannot rely on the SCB status register as it
 * takes some time to update the bits after a command has been issued
 */

static inline void
clear_interrupt(Uint16 flags)
{
	base_address_16[SCB_STATUS_WORD] = flags;
}

static inline Uint16 
read_scb_status(void)
{
	return base_address_16[SCB_STATUS_WORD];
}

static inline void 
write_scb_command(Uint16 value)
{
	base_address_16[SCB_COMMAND_WORD] = value;
}

static void reset_e100(void)
{
	// Get the base memory address
	base_address_8  = (Uint8  *)pci_e100->type_0.bar_address[0];
	base_address_16 = (Uint16 *)pci_e100->type_0.bar_address[0];
	base_address_32 = (Uint32 *)pci_e100->type_0.bar_address[0];

	// Set the cache line size
//	base_address_8[CACHE_LINE_SIZE] = 0x10; // 16


	
	
	__install_isr(pci_e100->type_0.interrupt_pin, __isr_nic_card);		
}

void __net_init(void)
{
	pci_e100 = __pci_find_by_device(E100_VENDOR, E100_DEVICE);

	if (pci_e100 == NULL)
	{
		_kpanic("Network", "Couldn't find network device!\n", 0);
	}

	serial_printf("Network device found!\n");

	/* Print out the sizes of our BAR addresses */
	for (int i = 0; i < 6; ++i)
	{
		serial_printf("Bar #%d Requires 0x%x bytes\n", i, pci_e100->type_0.bar_sizes[i]);
	}
	
	reset_e100();
}
