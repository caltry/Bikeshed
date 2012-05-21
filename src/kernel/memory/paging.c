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


#define ADDR_TO_PD_IDX(x) ((x) >> 22)
#define ADDR_TO_PT_IDX(x) (((x) >> 12) & 0x03FF) 
#define PAGE_DIR_ADDR 0xFFFFF000
#define PAGE_TBL_FROM_ADDR(x) (((Uint32 *)0xFFC00000) + (0x400 * (((Uint32)(x)) >> 22)))
#define PAGE_TBL_FROM_INDEX(x) (((Uint32 *)0xFFC00000) + (0x400 * (x)))

/* Similar to unmap page except it DOESN'T free the page from the physical memory
 * manager.
 *
 * This is required when copying pages because if we're trying to create a new page
 * table we don't want to free it's blocks before it gets used!
 */
static void __virt_clear_page(void *virtual_addr);


// TODO change this to directly link to the already defined page directory
page_directory_t *__virt_kpage_directory = 0;//&BootPageDirectory;

static Uint32 KERNEL_PAGE_DIR_INDEX     = 0;
static Uint32 KERNEL_PAGE_DIR_INDEX_END = 0;
static Uint32 KERNEL_PAGE_TBL_START = 0;
static Uint32 KERNEL_PAGE_TBL_END   = 0;

// Used for cloning page directories
static Uint32* scratch_pd;
static Uint32* scratch_pt;
static Uint32* scratch_pte;

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

	// Used later when copying pages and page tables
	scratch_pd  = (Uint32 *)(((KERNEL_LINK_ADDR+KERNEL_SIZE) & 0xFFFFF000) + 2*PAGE_SIZE);
	scratch_pt  = (Uint32 *)(((void *)scratch_pd) + PAGE_SIZE);
	scratch_pte = (Uint32 *)(((void *)scratch_pt) + PAGE_SIZE);

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
	if (pd[page_dir_index] == 0)
	{
		return (void *)0xFFFFFFFF;
	}
	

	Uint32 *pt = ((Uint32*)0xFFC00000) + (0x400 * page_dir_index);
	if (pt[page_tbl_index] == 0)
	{
		return (void *)0xFFFFFFFF;
	}
	// TODO Here check whether the PT is present
	
	return (void *)((pt[page_tbl_index] & ~0xFFF) + ((Uint32)virtual_addr & 0xFFF));
}

void __virt_reset_page_directory()
{
	for (void* address = (void *)0x400000; address < (void *)0xC0000000; address += PAGE_SIZE)
	{
		__virt_unmap_page(address);
	}
}

void __virt_dealloc_page_directory()
{
	// Frees all pages after 1MiB and before the kernel's pages */
	__virt_reset_page_directory();

	Uint32* pd = (Uint32 *)0xFFFFF000;
	__phys_unset_bit((void *)(pd[1023])); /* Finally free the block of memory this page directory occupies */
}

page_directory_t* __virt_clone_directory()
{
	// We need some scratch space while we're copying the currenty directory	
	serial_printf("Cloning directory\n");

	__virt_clear_page(scratch_pd);
	__virt_clear_page(scratch_pt);
	__virt_clear_page(scratch_pte);

	void* phys_pd_address = __phys_get_free_4k();

	__virt_map_page(phys_pd_address, scratch_pd, PG_READ_WRITE);

	Uint32* kernel_pd = (Uint32 *)PAGE_DIR_ADDR;
	Uint32* kernel_pt = (Uint32 *)PAGE_TBL_FROM_INDEX(0);

	// Identity map the lowest 4MiB - TODO expand to just 1MiB
	scratch_pd[0] = kernel_pd[0];

	// Identity map the rest of the kernel
	for (Uint32 i = KERNEL_PAGE_DIR_INDEX; i < 1023; ++i)
	{
		scratch_pd[i] = kernel_pd[i];
	}

	// Now we need to copy what's inside of the user space portion	
	void* address = (void *)0x400000;
	if (ADDR_TO_PD_IDX((Uint32)address) != 1) { _kpanic("CLONE DIRECTORY", "BAD INDEX", 0); }	

	for (Uint32 dir_index = ADDR_TO_PD_IDX((Uint32)address); dir_index < KERNEL_PAGE_DIR_INDEX; ++dir_index)
	{
		if (kernel_pd[dir_index] != 0)
		{
			serial_printf("---Kernel pd: %d - non-empty\n");
			kernel_pt = PAGE_TBL_FROM_INDEX(dir_index);
			
			__virt_clear_page(scratch_pt);
			void* phys_pt_address = __phys_get_free_4k();
			__virt_map_page(phys_pt_address, scratch_pt, PG_READ_WRITE);

			scratch_pd[dir_index] = (Uint32)phys_pt_address | (kernel_pd[dir_index] & 0xFFF); // Copy the kernel's flags

			for (Uint32 j = 0; j < 1024; ++j, address += PAGE_SIZE)
			{
				if (kernel_pt[j] != 0)
				{
					__virt_clear_page(scratch_pte);
					void* phys_pte_address = __phys_get_free_4k();
					__virt_map_page(phys_pte_address, scratch_pte, PG_READ_WRITE);

					scratch_pt[j] = (Uint32)phys_pte_address | (kernel_pt[j] & 0xFFF); // Copy the kernel's flags

					// Physically copy the addresses
					_kmemcpy(scratch_pte, address, PAGE_SIZE);
				}
			}
		} else {
			address += PAGE_SIZE*1024;
		}
	}

	// Map the new page directory back to itself
	scratch_pd[1023] = (Uint32)phys_pd_address | PG_READ_WRITE | PG_PRESENT;

	// Unmap all the temporary pages, All changes to kernel tables affect all processes
	__virt_clear_page(scratch_pd);
	__virt_clear_page(scratch_pt);
	__virt_clear_page(scratch_pte);

	return phys_pd_address;
}

void __virt_clear_page(void *virtual_addr)
{
	if ((virtual_addr < (void *)0x100000 || virtual_addr >= (void *)KERNEL_LINK_ADDR) &&
			virtual_addr != scratch_pd && virtual_addr != scratch_pt && virtual_addr != scratch_pte)
	{
		_kpanic("Clear Page", "Tried to free a reserved address!", 0);
	}

	Uint32 page_dir_index = (Uint32)virtual_addr >> 22;
	Uint32 page_tbl_index = (Uint32)virtual_addr >> 12 & 0x03FF;

	Uint32 *pd = (Uint32 *)0xFFFFF000;
	if ((pd[page_dir_index] & PG_PRESENT) == 0)
	{
		return;
	}

	Uint32 *pt = ((Uint32 *)0xFFC00000) + (0x400 * page_dir_index);
	if (pt[page_tbl_index] != 0)
	{
		pt[page_tbl_index] = 0;
		serial_printf("---Clear: Removing page table entry: %d - %x\nChecking if page table is empty\n", page_tbl_index, virtual_addr);

		Uint32 i = 0;
		for (; i < 1024; ++i)
		{
			if (pt[i] != 0) { break; }
		}

		if (i == 1024)
		{
			serial_printf("================================================CLEAR PAGE: FREEING PDE\n");
			pd[page_dir_index] = 0;
		}
	}

	asm volatile("invlpg %0"::"m" (*(char *)((Uint32)virtual_addr & 0xFFFFF000)));	
	asm volatile("invlpg %0"::"m" (*(char *)((Uint32)pt & 0xFFFFF000)));	
}

void __virt_unmap_page(void *virtual_addr)
{
	if (virtual_addr < (void *)0x400000 || virtual_addr >= (void *)0xC0000000)
	{
		_kpanic("Unmap Page", "Tried to free a reserved address!", 0);
	}

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
		serial_printf("Removing page directory: %x\n", pd[page_dir_index]);
		//pd[page_dir_index] &= ~PG_PRESENT;
		__phys_unset_bit((void *)pd[page_dir_index]);
		pd[page_dir_index] = 0;
	}

	// Now you need to flush the entry in the TBL
	// or you might not notice the change
	// Is this the virtual address or the physical address?
	asm volatile("invlpg %0"::"m" (*(char *)(virtual_addr)));	
	asm volatile("invlpg %0"::"m" (*(char *)((Uint32)pt & 0xFFFFF000)));	
}

void __virt_map_page(void *physical_addr, void *virtual_addr, Uint32 flags)
{
	// Make sure both addresses are page aligned
	
	Uint32 page_dir_index = (Uint32)virtual_addr >> 22;
	Uint32 page_tbl_index = (Uint32)virtual_addr >> 12 & 0x03FF;

	Uint32 *pd = (Uint32 *)0xFFFFF000;

	serial_printf("Mapping page: %x - to - %x\n", physical_addr, virtual_addr);
	serial_printf("Page dir idx: %d - Page tbl idx: %d\n", page_dir_index, page_tbl_index);

	// Here check if the PD entry is present
	// When it's not present, create a new empty PT and adjust the PDE accordingly
	//serial_printf("Page dir value: %x\n", pd[page_dir_index]);
	Uint32 *pt = ((Uint32 *)0xFFC00000) + (0x400 * page_dir_index); 
	if ((pd[page_dir_index] & PG_PRESENT) == 0)
	{
		serial_string("Page directory entry not present!\n");		
		pd[page_dir_index] = (Uint32)__phys_get_free_4k() | PG_READ_WRITE | PG_PRESENT;
		_kmemclr(pt, PAGE_SIZE);
	}
	
	// Here need to check whether the PT entry is present
	// When it is, then there is already a mapping present, what do you do?
	if (pt[page_tbl_index] != 0)
	{
		// Uh-oh... This shouldn't happen!
		serial_printf("Phys: %x\n", (Uint32)physical_addr);
		serial_printf("Virt: %x\n", (Uint32)virtual_addr);
		serial_printf("Dir Index: %d\n", page_dir_index);
		serial_printf("Tbl Index: %d\n", page_tbl_index);

		for (int i = 0; i < 1024; ++i)
		{
			serial_printf("i: %d - Value: %x\n", i, pt[i]);
		}

		c_printf("Phys: %x - Virt: %x\n", physical_addr, virtual_addr);
		c_printf("Dir IDX: %d - Tbl IDX: %d\n", page_dir_index, page_tbl_index);
		_kpanic("Paging", "A page has already been mapped here!\n", 0);
	}
	
	pt[page_tbl_index] = ((Uint32)physical_addr) | (flags & 0xFFF) | PG_PRESENT; // Present

	// Now you need to flush the entry in the TBL
	// or you might not notice the change
	// Is this the virtual address or the physical address?
	asm volatile("invlpg %0"::"m" (*(char *)(virtual_addr)));	
	asm volatile("invlpg %0"::"m" (*(char *)((Uint32)pt & 0xFFFFF000)));	
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
	serial_printf("Offending process: %d\n", _current->pid);
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
