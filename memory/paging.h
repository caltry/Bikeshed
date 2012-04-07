#ifndef __PAGING_H__
#define __PAGING_H__

#include "defs.h"
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
extern page_directory_t BootPageDirectory;

void __virt_initialize_paging(void);

void __virt_switch_page_directory(page_directory_t *page_directory);

// Taken from OS Dev wiki
void* __virt_get_phys_addr(void *virtual_addr);

// Taken from OS Dev wiki
void __virt_map_page(void *physical_addr, void *virtual_addr, uint32 flags);

void __virt_unmap_page(void *virtual_addr);
#endif
