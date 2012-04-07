#include "utils.h"
#include "paging.h"
#include "physical.h"
#include "../serial.h"

// TODO change this to directly link to the already defined page directory
page_directory_t *__virt_kpage_directory = 0;//&BootPageDirectory;

void __virt_initialize_paging()
{
	serial_printf("Boot page dir location: %x\n", &BootPageDirectory);
	__virt_kpage_directory = (page_directory_t *)__phys_get_free_4k();
	serial_printf("Page dir location: %x\n", __virt_kpage_directory);

	page_table_t *first_table  = (page_table_t *)__phys_get_free_4k();
	page_table_t *kernel_table = (page_table_t *)__phys_get_free_4k();
	serial_printf("First table: %x\n", first_table);
	serial_printf("Kernel table: %x\n", kernel_table);

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

	serial_printf("Kernel pages: %d\n", pages);
	serial_printf("Kernel page_dir_index: %d\n", page_dir_index);
	serial_printf("Kernel end: %d\n", page_dir_end_index);
	serial_printf("Page table start: %d\n", page_table_start);
	serial_printf("Page table end: %d\n", page_table_end);

	i = page_table_start;
	address = KERNEL_LOAD_ADDR;
	for (; i < page_table_end; ++i)
	{
		kernel_table->pages[i].value = address | 3;
		address += 4096;
	}

	__virt_kpage_directory->ptables[0] = (uint32)first_table | 3;
	__virt_kpage_directory->ptables[page_dir_index] = (uint32)kernel_table | 3;

	// Set the last entry to ourself so we can do fancy reading of the page
	// directory from inside of virtual memory space
	__virt_kpage_directory->ptables[1023] = (uint32)__virt_kpage_directory | 3;

	__virt_switch_page_directory(__virt_kpage_directory);

	serial_string("4KB page tables enabled\n");

	uint32* data = (uint32 *)__phys_get_free_4k();
	_kmemset(data, 0, 4096);
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

	uint32 *pt = ((uint32 *)0xFFC00000) + (0x400 * page_dir_index);
	// Here need to check whether the PT entry is present
	// When it is, then we need to unmap it

	pt[page_tbl_index] = 0;

	// TODO check if Page Table is empty, and mark the Page Directory entry
	// as empty

	// Now you need to flush the entry in the TBL
	// or you might not notice the change
}

void __virt_map_page(void *physical_addr, void *virtual_addr, uint32 flags)
{
	// Make sure both addresses are page aligned
	
	uint32 page_dir_index = (uint32)virtual_addr >> 22;
	uint32 page_tbl_index = (uint32)virtual_addr >> 12 & 0x03FF;

	uint32 *pd = (uint32 *)0xFFFFF000;

	// TODO Here check if the PD entry is present
	// When it's not present, create a new empty PT and adjust the PDE accordingly
	
	uint32 *pt = ((uint32 *)0xFFC00000) + (0x400 * page_dir_index); 
	// TODO Here need to check whether the PT entry is present
	// When it is, then there is already a mapping present, what do you do?
	
	pt[page_tbl_index] = ((uint32)physical_addr) | (flags & 0xFFF) | 0x01; // Presnt

	// TODO Now you need to flush the entry in the TBL
	// or you might not notice the change
}

void __virt_switch_page_directory(page_directory_t *page_directory)
{
	// TODO make this a macro?
	asm volatile("mov %0, %%cr3" :: "b"(page_directory));
	// TODO this stuff isn't needed once paging is enabled
	unsigned int cr0;
	asm volatile("mov %%cr0, %0": "=b"(cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %0, %%cr0":: "r"(cr0));
}
