#include "paging.h"
#include "physical.h"
#include "serial.h"

#include "x86arch.h"
#include "../lib/klib.h"
#include "../../ulib/c_io.h"

typedef long unsigned int ul;

void _kmemset(void *ptr, unsigned char value, uint32 num)
{
	unsigned char *p = (unsigned char *)ptr;
	while (num > 0)
	{
		*p = value;
		++p;
		--num;
	}
}

// TODO change this to directly link to the already defined page directory
page_directory_t *__virt_kpage_directory = 0;//&BootPageDirectory;

void __virt_initialize_paging()
{
	serial_printf("Boot page dir location: %x\n", (ul)&boot_page_directory);
	__virt_kpage_directory = (page_directory_t *)__phys_get_free_4k();
	serial_printf("Page dir location: %x\n", (ul)__virt_kpage_directory);

	page_table_t *first_table  = (page_table_t *)__phys_get_free_4k();
	page_table_t *kernel_table = (page_table_t *)__phys_get_free_4k();
	serial_printf("First table: %x\n", (ul)first_table);
	serial_printf("Kernel table: %x\n", (ul)kernel_table);

	_kmemset(__virt_kpage_directory, 0, sizeof(page_directory_t));
	_kmemset(first_table,  0, sizeof(page_table_t));
	_kmemset(kernel_table, 0, sizeof(page_table_t));
	
	// Identity map the lower 1MB of pages
	uint32 i = 0;
	uint32 address = 0;
	for (; i < 256; ++i)
	{
		first_table->pages[i].value = address | 3;
		address += 4096;
	}

	// Map the kernel
	//address = KERNEL_LOAD_ADDR;
	uint32 pages = KERNEL_SIZE / 4096;
	uint32 page_dir_index = KERNEL_LINK_ADDR / (4096 * 1024);
	uint32 page_dir_end_index = (KERNEL_LINK_ADDR + KERNEL_SIZE) / (4096 * 1024);

	uint32 page_table_start = (KERNEL_LINK_ADDR >> 12) % 4096;
	uint32 page_table_end = ((KERNEL_LINK_ADDR + KERNEL_SIZE) >> 12) % 4096;

	serial_printf("Kernel pages: %d\n", (ul)pages);
	serial_printf("Kernel page_dir_index: %d\n", (ul)page_dir_index);
	serial_printf("Kernel end: %d\n", (ul)page_dir_end_index);
	serial_printf("Page table start: %d\n", (ul)page_table_start);
	serial_printf("Page table end: %d\n", (ul)page_table_end);

	/* We don't want to overwrite our 1MB identity mapping
	 */
	i = page_table_start;
	address = KERNEL_LOAD_ADDR;
	if (page_dir_index != 0)
	{
		serial_string("Kernel is not in the lowest 4MB\n");
		//for (; i < page_table_end; ++i)
		for (; i < page_table_end+1; ++i)
		{
			kernel_table->pages[i].value = address | 3;
			address += 4096;
		}

		__virt_kpage_directory->ptables[page_dir_index] = (uint32)kernel_table | 3;
	} else {
		serial_string("Kernel is in the lowest 4MB\n");
		for (; i < page_table_end; ++i)
		{
			first_table->pages[i].value = address | 3;
			address += 4096;
		}

		__phys_unset_bit((void *)kernel_table);
	}

	serial_string("Installing page fault handler\n");

	// Install the page fault ISR
	__install_isr(INT_VEC_PAGE_FAULT, _isr_page_fault);

	serial_string("Setting up page mappings\n");
	// Add the < 1MB identity mapping
	__virt_kpage_directory->ptables[0] = (uint32)first_table | 3;

	// Map the page directory back to itself for easy modification
	__virt_kpage_directory->ptables[1023] = (uint32)__virt_kpage_directory | 3;

	// Set the last entry to ourself so we can do fancy reading of the page
	// directory from inside of virtual memory space
	__virt_switch_page_directory(__virt_kpage_directory);

	serial_string("4KB page tables enabled\n");

	//uint32* data = (uint32 *)__phys_get_free_4k();
	//_kmemset(data, 0, 4096);
	c_puts("Paging Enabled\n");
}

void* __virt_get_phys_addr(void *virtual_addr)
{
	uint32 page_dir_index = (uint32)virtual_addr >> 22;
	uint32 page_tbl_index = (uint32)virtual_addr >> 12 & 0x03FF;

	uint32 *pd = (uint32 *)0xFFFFF000;

	// TODO Check whether or not the page directory is present
	

	uint32 *pt = ((uint32*)0xFFC00000) + (0x400 * page_dir_index);
	// TODO Here check whether the PT is present
	
	return (void *)((pt[page_tbl_index] & ~0xFFF) + ((uint32)virtual_addr & 0xFFF));
}

void __virt_unmap_page(void *virtual_addr)
{
	uint32 page_dir_index = (uint32)virtual_addr >> 22;
	uint32 page_tbl_index = (uint32)virtual_addr >> 12 & 0x03FF;

	uint32 *pd = (uint32 *)0xFFFFF000;
	
	// Here check if the PD entry is present
	// When it's not present, we're done
	if ((pd[page_dir_index] & PRESENT) == 0)
	{
		return;
	}

	uint32 *pt = ((uint32 *)0xFFC00000) + (0x400 * page_dir_index);
	// Here need to check whether the PT entry is present
	// When it is, then we need to unmap it
	if ((pt[page_tbl_index] & PRESENT) > 0)
	{
		// Unmap the physical address 
		__phys_unset_bit((void *)pt[page_tbl_index]);
	}

	pt[page_tbl_index] = 0;

	// check if Page Table is empty, and mark the Page Directory entry
	// as empty
	Uint32 i = 0;
	for (; i < 1024; ++i)
	{
		if ((pt[i] & PRESENT) > 0)
		{
			break;
		}
	}

	if (i == 1024)
	{
		pd[page_dir_index] &= ~PRESENT;
		__phys_unset_bit((void *)pd[page_dir_index]);
	}

	// Now you need to flush the entry in the TBL
	// or you might not notice the change
	// Is this the virtual address or the physical address?
	asm volatile("invlpg %0"::"m" (*(char *)virtual_addr));	
}

void __virt_map_page(void *physical_addr, void *virtual_addr, uint32 flags)
{
	// Make sure both addresses are page aligned
	
	uint32 page_dir_index = (uint32)virtual_addr >> 22;
	uint32 page_tbl_index = (uint32)virtual_addr >> 12 & 0x03FF;

	uint32 *pd = (uint32 *)0xFFFFF000;

	serial_string("Mapping page 1/4\n");
	serial_printf("Phys: %x\n", (Uint32)physical_addr);
	serial_printf("Virt: %x\n", (Uint32)virtual_addr);
	serial_printf("Dir Index: %d\n", page_dir_index);
	serial_printf("Tbl Index: %d\n", page_tbl_index);

	// Here check if the PD entry is present
	// When it's not present, create a new empty PT and adjust the PDE accordingly
	serial_printf("Page dir value: %x\n", pd[page_dir_index]);
	if ((pd[page_dir_index] & PRESENT) == 0)
	{
		serial_string("Page directory entry not present!\n");		
		pd[page_dir_index] = (Uint32)__phys_get_free_4k();
		_kmemset(&pd[page_dir_index], 0, 4096);
		pd[page_dir_index] |= READ_WRITE | PRESENT;
	}
	
	uint32 *pt = ((uint32 *)0xFFC00000) + (0x400 * page_dir_index); 
	serial_string("Mapping page 2/4\n");
	// Here need to check whether the PT entry is present
	// When it is, then there is already a mapping present, what do you do?
	if ((pt[page_tbl_index] & PRESENT) > 0)
	{
		// Uh-oh... This shouldn't happen!
		_kpanic("Paging", "A page has already been mapped here!\n", 0);
	}
	
	serial_string("Mapping page 3/4\n");
	pt[page_tbl_index] = ((uint32)physical_addr) | (flags & 0xFFF) | PRESENT; // Present

	serial_string("Mapping page 4/4\n");
	// Now you need to flush the entry in the TBL
	// or you might not notice the change
	// Is this the virtual address or the physical address?
	asm volatile("invlpg %0"::"m" (*(char *)virtual_addr));	
}

void __virt_switch_page_directory(page_directory_t *page_directory)
{
	serial_printf("Switching page directory %x\n", (Uint32)page_directory);
	// TODO make this a macro?
	asm volatile("mov %0, %%cr3" :: "b"(page_directory));
	// TODO this stuff isn't needed once paging is enabled
	unsigned int cr0;
	asm volatile("mov %%cr0, %0": "=b"(cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %0, %%cr0":: "r"(cr0));
	serial_string("Done switching page directory\n");
}

static const char* const page_table_errors[] = {
	"Supervisory process tried to read a non-present page entry",
	"Supervisory process tried to read a page and caused a protection fault",
	"Supervisory process tried to write to a non-present page entry",
	"Supervisory process tried to write a page and caused a protection fault",
	"User process tried to read a non-present page entry",
	"User process tried to read a page and caused a protection fault",
	"User process tried to write to a non-present page entry",
	"User process tried to write to a non-present page entry"
};

void _isr_page_fault(int vector, int code)
{
	serial_string("US RW P - Description\n");
	serial_printf("%d  ",  code & 0x4);
	serial_printf("%d  ",  code & 0x2);
	serial_printf("%d - ", code & 0x1);
	serial_string(page_table_errors[code & 0x7]);
	serial_string("\n");
	unsigned int cr2 = read_cr2();
	serial_printf("Address: %X\n", cr2);
	serial_printf("Page Directory Entry: %d\n", (cr2 >> 22));
	serial_printf("Page Table Entry:     %d\n", (cr2 >> 12) & 0x3FF);

	_kpanic("Paging", "Page fault handling not fully implemented!", 0);
}
