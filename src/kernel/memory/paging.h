#ifndef __PAGING_H__
#define __PAGING_H__

#include "../../include/defs.h"
/*
typedef struct Page
{
	uint32 present 		: 1;
	uint32 read_write 	: 1;
	uint32 user 		: 1;
	uint32 accessed 	: 1;
	uint32 dirty 		: 1;
	uint32 unused 		: 7;
	uint32 frame 		: 20;
} page_t;
*/
#define PRESENT 	0x1
#define READ_WRITE 	0x2
#define USER 		0x4
#define WRITE_THRU	0x8
#define CACHE_DISABLE 0x10

typedef struct Page 
{
	uint32 value;
} page_t;

typedef struct PageTable
{
	page_t pages[1024];
} page_table_t;

typedef union PageDirectory
{
	page_table_t *tables[1024];
	uint32 ptables[1024];
} page_directory_t;

page_directory_t *__virt_kpage_directory;
extern page_directory_t boot_page_directory;

/* Initializes paging for our operating system. It identity maps the
 * first 1MiB of memory meaning the linear and virtual addresses are
 * the same.
 *
 * Next it looks at the kernels linked location (defined in physical.h)
 * and finds the appropriate place in the kernel page directory for
 * mapping the kernel pages. The number of pages is KERNEL_SIZE / 4096
 *
 * By default with our higher half kernel the page directory looks like
 * the following
 *
 * Key:
 * R/W = Read/Write
 * S   = Supervisor
 * P   = Present
 *
 * Index    Page directory     Page Table [index1-index2] == physical address1 - physical address2
 *        +---------------+
 *	0     |    R/W S P    | -> [0-256] == 0x0 - 0x100000 R/W S, [256-1023] == 0
 *        +---------------+
 * 1-767  |  Not mapped   |
 *        +---------------+
 *  768   |    R/W S P    | -> [16-K_END] == 0x100000 - KERNEL_SIZE R/W S, [KERNEL_SIZE-1023] == 0
 *        +---------------+
 * 769-   |  Not Mapped   |
 * 1023   |               |
 *        +---------------+
 */
void __virt_initialize_paging(void);

/* Changes the current page directory used for virtual memory. This is useful
 * when context switching so each process can have it's own address space.
 *
 * It's really just a wrapper around an assembly call:
 *
 *    asm volatile("mov %0, %%cr3" :: "b"(page_directory));
 */
void __virt_switch_page_directory(page_directory_t *page_directory);

// Taken from OS Dev wiki
void* __virt_get_phys_addr(void *virtual_addr);

// Taken from OS Dev wiki
void __virt_map_page(void *physical_addr, void *virtual_addr, uint32 flags);

/* Do the reverse of __virt_map_page(). If there is an entry present
 * it will set it to not present and if all the page table entries are
 * marked not present it will free the physical address used by the
 * page table. Finally it invalidates the virtual address in the TLB.
 *
 * The address supplied is the virtual address to unmap
 *
 * Only 4KiB aligned virtual addresses will have any effect.
 */
void __virt_unmap_page(void *virtual_addr);

/* Used to handle page faults. Currently it displays what the
 * faulting address was and what the error bits were and then
 * does a kernel panic.
 */
void _isr_page_fault(int vector, int code);

/* Defined in memory.S */
Uint32 read_cr0(void);
void write_cr0(Uint32 address);

Uint32 read_cr3(void);
void write_cr3(Uint32 address);

/* Used by the page fault handler to get the offending address */
Uint32 read_cr2(void);

#endif
