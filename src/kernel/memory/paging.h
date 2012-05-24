#ifndef __PAGING_H__
#define __PAGING_H__

#include "types.h"

/* Some basic defines to help out in the code */
#define PAGE_SIZE 4096

#define PG_PRESENT 		 0x1
#define PG_READ_WRITE 	 0x2
#define PG_USER 		 0x4
#define PG_WRITE_THRU	 0x8
#define PG_CACHE_DISABLE 0x10

#define ADDR_TO_PD_IDX(x) ((x) >> 22)
#define ADDR_TO_PT_IDX(x) (((x) >> 12) & 0x03FF) 
#define PAGE_DIR_ADDR 0xFFFFF000
#define PAGE_TBL_FROM_ADDR(x) (((Uint32 *)0xFFC00000) + (0x400 * (((Uint32)(x)) >> 22)))
#define PAGE_TBL_FROM_INDEX(x) (((Uint32 *)0xFFC00000) + (0x400 * (x)))

/* Forward declarations */
struct Page;
typedef struct Page page_t;

struct PageTable;
typedef struct PageTable page_table_t;

union PageDirectory;
typedef union PageDirectory page_directory_t;

extern page_directory_t *__virt_kpage_directory;
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
 * 769-   |  Not Mapped   | -> These regions are mapped by other modules in the kernel like
 * 1023   |               |    the filesystem or video module
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

/* Gets the physical address associated with a virtual address. 
 *
 * Since the last four megabytes of address space is mapped to the
 * start of the current page directory, this function doesn't need
 * to know what the current page directory is.
 *
 * Adapted from the OSdev wiki
 */
void* __virt_get_phys_addr(void* virtual_addr);

/* Clears everything but kernel entries from a page table
 *
 * This function internally calls unmap_page which will mark
 * any of the pages being used by the current page directory,
 * except for those used by the kernel.
 */
void __virt_reset_page_directory(void);

/* Removes the entire page table from memory 
 *
 * It's recommended that dispatch is called after this
 * function. Mainly used to cleanup processes page directories
 * when exiting
 */
void __virt_dealloc_page_directory(void);

/* Clones a page directory, it copies all pages and data 
 *
 * This function performs a dumb copy of another page directory.
 * It will use the same values for the page_table[0] and those
 * above page_table[kernel_start]. The pages in the middle are
 * physically copied to another page and them mapped into the
 * newly created page table.
 *
 * Returns the physical address of the new page table
 */
page_directory_t* __virt_clone_directory(void);

/* Maps a physical address to a virtual address. Creates a new
 * page directory entry if one doesn't exist. You don't need to
 * pass in PG_PRESENT as it's automatically OR'd into the flags.
 *
 * Adapted from the OSdev wiki
 */
void __virt_map_page(void* physical_addr, void* virtual_addr, Uint32 flags);

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

/* Similar to unmap page except it DOESN'T free the page from the physical memory
 * manager.
 *
 * This is required when copying pages because if we're trying to create a new page
 * table we don't want to free it's blocks before it gets used!
 */
void __virt_clear_page(void *virtual_addr);

/* Used to handle page faults. Currently it displays what the
 * faulting address was and what the error bits were and then
 * does a kernel panic.
 */
void _isr_page_fault(int vector, int code);

/* Defined in memory.S 
 *
 * These are simple getters and setters for the registers
 */
Uint32 read_cr0(void);
void write_cr0(Uint32 address);

Uint32 read_cr3(void);
void write_cr3(Uint32 address);

/* Used by the page fault handler to get the offending address */
Uint32 read_cr2(void);

#endif
