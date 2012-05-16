#include "defs.h"
#include "paging.h"
#include "physical.h"
#include "serial.h"
#include "x86arch.h"
#include "scheduler.h"
#include "support.h"
#include "c_io.h"

//#include "../lib/klib.h"
#include "lib/klib.h"

struct Page 
{
	Uint32 value;
};

struct PageTable
{
	page_t pages[1024];
};

union PageDirectory
{
	page_table_t *tables[1024];
	Uint32 ptables[1024];
};

typedef long unsigned int ul;

void _kmemset(void *ptr, unsigned char value, Uint32 num)
{
	unsigned char *p = (unsigned char *)ptr;
	while (num > 0)
	{
		*p = value;
		++p;
		--num;
	}
}

#define ADDR_TO_PD_IDX(x) ((x) >> 22)
#define ADDR_TO_PT_IDX(x) (((x) >> 12) & 0x03FF) 

// TODO change this to directly link to the already defined page directory
page_directory_t *__virt_kpage_directory = 0;//&BootPageDirectory;

static Uint32 KERNEL_PAGE_DIR_INDEX     = 0;
static Uint32 KERNEL_PAGE_DIR_INDEX_END = 0;
static Uint32 KERNEL_PAGE_TBL_START = 0;
static Uint32 KERNEL_PAGE_TBL_END   = 0;

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
	Uint32 i = 0;
	Uint32 address = 0;
	for (; i < 256; ++i)
	{
		first_table->pages[i].value = address | 3;
		address += PAGE_SIZE;
	}

	// Map the kernel
	Uint32 pages = KERNEL_SIZE / PAGE_SIZE;
	KERNEL_PAGE_DIR_INDEX = KERNEL_LINK_ADDR / (PAGE_SIZE * 1024);
	KERNEL_PAGE_DIR_INDEX_END = (KERNEL_LINK_ADDR + KERNEL_SIZE) / (PAGE_SIZE * 1024);

	KERNEL_PAGE_TBL_START = (KERNEL_LINK_ADDR >> 12) % PAGE_SIZE;
	KERNEL_PAGE_TBL_END = (((KERNEL_LINK_ADDR + KERNEL_SIZE) >> 12) % PAGE_SIZE) + 1;

	serial_printf("Kernel pages: %d\n", pages);
	serial_printf("Kernel page_dir_index: %d\n", KERNEL_PAGE_DIR_INDEX);
	serial_printf("Kernel end: %d\n", KERNEL_PAGE_DIR_INDEX_END);
	serial_printf("Page table start: %d\n", KERNEL_PAGE_TBL_START);
	serial_printf("Page table end: %d\n", KERNEL_PAGE_TBL_END);

	/* We don't want to overwrite our 1MB identity mapping
	 */
	i = KERNEL_PAGE_TBL_START;
	address = KERNEL_LOAD_ADDR;
	if (KERNEL_PAGE_DIR_INDEX != 0)
	{
		serial_string("Kernel is not in the lowest 4MB\n");
		for (; i < KERNEL_PAGE_TBL_END; ++i)
		{
			kernel_table->pages[i].value = address | 3;
			address += PAGE_SIZE;
		}

		__virt_kpage_directory->ptables[KERNEL_PAGE_DIR_INDEX] = (Uint32)kernel_table | 3;
	} else {
		serial_string("Kernel is in the lowest 4MB\n");
		for (; i < KERNEL_PAGE_TBL_END-1; ++i)
		{
			first_table->pages[i].value = address | 3;
			address += PAGE_SIZE;
		}

		__phys_unset_bit((void *)kernel_table);
	}

	serial_string("Installing page fault handler\n");

	// Install the page fault ISR
	__install_isr(INT_VEC_PAGE_FAULT, _isr_page_fault);

	serial_string("Setting up page mappings\n");
	// Add the < 1MB identity mapping
	__virt_kpage_directory->ptables[0] = (Uint32)first_table | 3;

	// Map the page directory back to itself for easy modification
	__virt_kpage_directory->ptables[1023] = (Uint32)__virt_kpage_directory | 3;

	// Set the last entry to ourself so we can do fancy reading of the page
	// directory from inside of virtual memory space
	__virt_switch_page_directory(__virt_kpage_directory);

	serial_string("4KB page tables enabled\n");
}

void* __virt_get_phys_addr(void *virtual_addr)
{
	Uint32 page_dir_index = (Uint32)virtual_addr >> 22;
	Uint32 page_tbl_index = (Uint32)virtual_addr >> 12 & 0x03FF;

	Uint32 *pd = (Uint32 *)0xFFFFF000;

	// TODO Check whether or not the page directory is present
	if ((pd[page_dir_index] & PG_PRESENT) > 0)
	{
		return (void *)0xFFFFFFFF;
	}
	

	Uint32 *pt = ((Uint32*)0xFFC00000) + (0x400 * page_dir_index);
	// TODO Here check whether the PT is present
	
	return (void *)((pt[page_tbl_index] & ~0xFFF) + ((Uint32)virtual_addr & 0xFFF));
}

void __virt_reset_page_directory(page_directory_t* page_directory)
{
	void* address = (void *)0x100000;
	for (; address < (void *)KERNEL_LINK_ADDR; address += PAGE_SIZE)
	{
		__virt_unmap_page(address);
	}

	// Dump the new address mapping
	Uint32* pd = (Uint32 *)0xFFFFF000;
	for (int i = 0; i < 1024; ++i)
	{
		serial_printf("\tIndex: %d - Value: %x\n", i, pd[i]);
		if (pd[i] != 0)
		{
			Uint32* pt = ((Uint32 *)0xFFC00000) + (0x400 * i); 
			for (int j = 0; j < 1024; ++j)
			{
				if (pt[j] != 0)
				{
					serial_printf("\t\tIndex: %d - Value: %x\n", j, pt[j]);
				}
			}
		}
	}
}

void __virt_dealloc_page_directory(page_directory_t* page_directory)
{
	// Frees all pages after 1MiB and before the kernel's pages */
	__virt_reset_page_directory(page_directory);

	Uint32* pd = (Uint32 *)0xFFFFF000;
	__phys_unset_bit((void *)(*pd)); /* Finally free the block of memory this page directory occupies */
}

page_directory_t* __virt_clone_directory(page_directory_t* page_directory)
{
	serial_printf("Cloning directory\n");
	// Need to somehow create a new page directory and clone it
	// We need some scratch space while we're copying the currenty directory	
	Uint32* scratch_pd  = (Uint32 *)(((KERNEL_LINK_ADDR+KERNEL_SIZE) & 0xFFFFF000) + 2*PAGE_SIZE);
	Uint32* scratch_pt  = (Uint32 *)(((void *)scratch_pd) + PAGE_SIZE);
	Uint32* scratch_pte = (Uint32 *)(((void *)scratch_pt) + PAGE_SIZE);

	// Allocate the new page directory
	void* phys_pd_address = __phys_get_free_4k();
	__virt_map_page(phys_pd_address, scratch_pd, PG_READ_WRITE);
	_kmemclr(scratch_pd, PAGE_SIZE);

	serial_printf("Assigned scratch_pd\n");

	// Allocate a page table for below 1MiB in case there are addresses being used
	// from 1MiB-4MiB
	void* phys_pt_address = __phys_get_free_4k();
	__virt_map_page(phys_pt_address, scratch_pt, PG_READ_WRITE);
	_kmemclr(scratch_pt, PAGE_SIZE);

	serial_printf("Assigned scratch_pt\n");

	// Now identity map the first 1MiB of addresses
	void* address = (void *)0x0;
	for (Int32 i = 0; i < 256; ++i, address += PAGE_SIZE)
	{
		scratch_pt[i] = (Uint32)address | PG_PRESENT | PG_READ_WRITE;
	}

	scratch_pd[0] = (Uint32)phys_pt_address | PG_PRESENT | PG_READ_WRITE;

	serial_printf("Identity mapped the first 1MiB\n");

	// Now copy the kernel's pages over		
	// TODO - Go to the page table level and copy only whats needed
	Uint32* kernel_pd = (Uint32 *)0xFFFFF000;
	for (Int32 i = KERNEL_PAGE_DIR_INDEX; i < 1024; ++i)
	{
		scratch_pd[i] = kernel_pd[i];
	}

	serial_printf("Identity mapped the kernel\n");

	// Now copy + memcpy the rest of the pages in the middle
	// We have to find the used addresses		
	// TODO - handle below 1MiB special case
	Uint32 cur_address = 0x400000;
	Uint32 page_dir_idx = ADDR_TO_PD_IDX(cur_address);
	
	for (; page_dir_idx < KERNEL_PAGE_DIR_INDEX; ++page_dir_idx)
	{
		// Only copy the things we need
		if (kernel_pd[page_dir_idx] != 0)
		{
			// Make a new entry in the new page directory
			scratch_pd[page_dir_idx] = (Uint32)__phys_get_free_4k() | PG_READ_WRITE | PG_PRESENT;
			__virt_unmap_page(scratch_pt); // Change the current page table we're looking at
			__virt_map_page((void *)scratch_pd[page_dir_idx], scratch_pt, PG_READ_WRITE);
			_kmemclr(scratch_pt, PAGE_SIZE);

			// TODO - What about copying the first few bytes inside of the kernel's PT? Up to KERNEL_PAGE_TBL_START

			// Now we want to loop through this page table
			Uint32* kernel_pt = ((Uint32 *)0xFFC00000) + (0x400 * page_dir_idx); 
			for (Uint32 page_tbl_idx = ADDR_TO_PT_IDX(cur_address); page_tbl_idx < 1024; ++page_tbl_idx, cur_address += 4096)
			{
				if (kernel_pt[page_tbl_idx] != 0)
				{
					scratch_pt[page_tbl_idx] = (Uint32)__phys_get_free_4k() | PG_READ_WRITE | PG_PRESENT;
					__virt_unmap_page(scratch_pte); // Change the current page table entry we're looking at
					__virt_map_page((void *)scratch_pt[page_tbl_idx], scratch_pte, PG_READ_WRITE);

					// Copy from the current address into scratch_pte
					_kmemcpy(scratch_pte, (void *)cur_address, PAGE_SIZE);
				}
			}
		} else {
			cur_address += PAGE_SIZE * 1024; // Skip to the next 4MiB
		}
	}

	// Unmap all the temporary pages, All changes to kernel tables affect all processes
	__virt_unmap_page(scratch_pd);
	__virt_unmap_page(scratch_pt);
	__virt_unmap_page(scratch_pte);

	return phys_pd_address;
}

void __virt_unmap_page(void *virtual_addr)
{
	Uint32 page_dir_index = (Uint32)virtual_addr >> 22;
	Uint32 page_tbl_index = (Uint32)virtual_addr >> 12 & 0x03FF;

	Uint32 *pd = (Uint32 *)0xFFFFF000;
	
	// Here check if the PD entry is present
	// When it's not present, we're done
	if ((pd[page_dir_index] & PG_PRESENT) == 0)
	{
		return;
	}

	Uint32 *pt = ((Uint32 *)0xFFC00000) + (0x400 * page_dir_index);
	// Here need to check whether the PT entry is present
	// When it is, then we need to unmap it
	if ((pt[page_tbl_index] & PG_PRESENT) > 0)
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
		if ((pt[i] & PG_PRESENT) > 0)
		{
			break;
		}
	}

	if (i == 1024)
	{
		pd[page_dir_index] &= ~PG_PRESENT;
		__phys_unset_bit((void *)pd[page_dir_index]);
	}

	// Now you need to flush the entry in the TBL
	// or you might not notice the change
	// Is this the virtual address or the physical address?
	asm volatile("invlpg %0"::"m" (*(char *)virtual_addr));	
}

void __virt_map_page(void *physical_addr, void *virtual_addr, Uint32 flags)
{
	// Make sure both addresses are page aligned
	
	Uint32 page_dir_index = (Uint32)virtual_addr >> 22;
	Uint32 page_tbl_index = (Uint32)virtual_addr >> 12 & 0x03FF;

	Uint32 *pd = (Uint32 *)0xFFFFF000;
/*
	serial_string("Mapping page 1/4\n");

	*/
	// Here check if the PD entry is present
	// When it's not present, create a new empty PT and adjust the PDE accordingly
	//serial_printf("Page dir value: %x\n", pd[page_dir_index]);
	Uint32 *pt = ((Uint32 *)0xFFC00000) + (0x400 * page_dir_index); 
	if ((pd[page_dir_index] & PG_PRESENT) == 0)
	{
		serial_string("Page directory entry not present!\n");		
		pd[page_dir_index] = (Uint32)__phys_get_free_4k();
		pd[page_dir_index] |= PG_READ_WRITE | PG_PRESENT;
	//	serial_printf("Page dir value 2: %x\n", (Uint32)pd[page_dir_index]);
	//	serial_printf("Page dir addr  3: %x\n", (Uint32)&pd[page_dir_index]);
		//_kmemset(pt, 0, sizeof(page_table_t));
		_kmemclr(pt, PAGE_SIZE);
	}
	
	//serial_string("Mapping page 2/4\n");
	// Here need to check whether the PT entry is present
	// When it is, then there is already a mapping present, what do you do?
	if ((pt[page_tbl_index] & PG_PRESENT) > 0)
	{
		// Uh-oh... This shouldn't happen!
		serial_printf("Phys: %x\n", (Uint32)physical_addr);
		serial_printf("Virt: %x\n", (Uint32)virtual_addr);
		serial_printf("Dir Index: %d\n", page_dir_index);
		serial_printf("Tbl Index: %d\n", page_tbl_index);
		/*serial_printf("Value: %x\n", pt[page_tbl_index-1]);
		serial_printf("Value: %x\n", pt[page_tbl_index-2]);
		serial_printf("Value: %x\n", pt[page_tbl_index]);
		serial_printf("Value: %x\n", pt[page_tbl_index+1]);
		serial_printf("Value: %x\n", pt[page_tbl_index+2]);
		*/
		for (int i = 0; i < 1024; ++i)
		{
			serial_printf("i: %d - Value: %x\n", i, pt[i]);
		}
		_kpanic("Paging", "A page has already been mapped here!\n", 0);
	}
	
	//serial_string("Mapping page 3/4\n");
	pt[page_tbl_index] = ((Uint32)physical_addr) | (flags & 0xFFF) | PG_PRESENT; // Present

	//serial_string("Mapping page 4/4\n");
	// Now you need to flush the entry in the TBL
	// or you might not notice the change
	// Is this the virtual address or the physical address?
	asm volatile("invlpg %0"::"m" (*(char *)virtual_addr));	
}

void __virt_switch_page_directory(page_directory_t *page_directory)
{
	asm volatile("mov %0, %%cr3" :: "b"(page_directory));
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
	UNUSED(vector);
	serial_string("US RW P - Description\n");
	serial_printf("%d  ",  code & 0x4);
	serial_printf("%d  ",  code & 0x2);
	serial_printf("%d - ", code & 0x1);
	serial_string(page_table_errors[code & 0x7]);
	serial_string("\n");
	serial_printf("OFFENDING ADDRESS: %x\n", _current->context->eip);
	unsigned int cr2 = read_cr2();
	serial_printf("Address: %x\n", cr2);
	serial_printf("Page Directory Entry: %d\n", (cr2 >> 22));
	serial_printf("Page Table Entry:     %d\n", (cr2 >> 12) & 0x3FF);
	c_printf("Address: 0x%x\n", cr2);
	c_printf("Offender: 0x%x\n", _current->context->eip);
	c_printf("Page Directory Entry: %d\n", (cr2 >> 22));
	c_printf("Page Table Entry: %d\n", (cr2 >> 12) & 0x3FF);
	c_printf("%s\n", page_table_errors[code & 0x7]);

	_kpanic("Paging", "Page fault handling not fully implemented!", 0);
}
