#include "elf.h"
#include "defs.h"
#include "fs/ext2/ext2.h"
#include "fs/ext2/read_ext2.h"
#include "memory/kmalloc.h"
#include "memory/physical.h"
#include "memory/paging.h"
#include "lib/klib.h"
#include "boot/bootstrap.h"
#include "syscalls.h"
#include "pcbs.h"


Status _elf_load_from_file(Pcb* pcb, const char* file_name)
{
	if (pcb == NULL || file_name == NULL) 
	{
		return BAD_PARAM;
	}

	// Try to open the file
	Elf32_Ehdr* elf32_hdr = (Elf32_Ehdr *)__kmalloc(sizeof(Elf32_Ehdr));

	Uint bytes_read = 0;
	ext2_read_status ext2_status =
		ext2_raw_read(bikeshed_ramdisk_context, file_name, (void *)elf32_hdr, &bytes_read, 0, sizeof(Elf32_Ehdr));

	if (ext2_status != EXT2_READ_SUCCESS /* Couldn't read the file */
		   || bytes_read < sizeof(Elf32_Ehdr) /* Clearly not an ELF file */
		   || elf32_hdr->e_magic != ELF_MAGIC_NUM /* Need the magic number! */
		   || elf32_hdr->e_type != ET_EXEC /* Don't support relocatable or dynamic files yet */
		   || elf32_hdr->e_machine != EM_386 /* Make sure it's for our architecture */
		   || elf32_hdr->e_entry != 0x0 /* Need an entry point */
		   || elf32_hdr->e_version != EV_CURRENT /* We don't support extensions right now */
		   || elf32_hdr->e_phoff == 0 /* If there are no program headers, what do we load? */
		   || elf32_hdr->e_phnum == 0) /* ... */
		// || elf32_hdr->e_ehsize != sizeof(Elf32_Ehdr)) /* The header size should match our struct */
	{

		_kpanic("ELF", "Couldn't open file!\n", 0);
		// Problem opening the file
		__kfree(elf32_hdr);
		return BAD_PARAM;
	}

	if (sizeof(Elf32_Phdr) != elf32_hdr->e_phentsize)
	{
		_kpanic("ELF", "program header size is different!\n", 0);
	}

	/* Okay lets start reading in and setting up the ELF file */	
	// We need a new buffer of size of (e_phentsize * e_phnum)
	Uint32 pheader_tbl_size = sizeof(Elf32_Phdr) * elf32_hdr->e_phnum;
	Elf32_Phdr* pheaders = (Elf32_Phdr *)__kmalloc(pheader_tbl_size);

	ext2_status = ext2_raw_read(bikeshed_ramdisk_context, file_name, (void *)pheaders, 
			&bytes_read, elf32_hdr->e_phoff, pheader_tbl_size);

	if (ext2_status != EXT2_READ_SUCCESS
			|| bytes_read < pheader_tbl_size)
	{
		_kpanic("ELF", "error reading file!\n", 0);
		__kfree(pheaders);
		__kfree(elf32_hdr);
		return BAD_PARAM;
	}

	// Cleanup the old processes page directory, we're replacing everything
	__virt_reset_page_directory(pcb->page_directory);

	/* We need to load all of the program sections now */
	for (Int32 i = 0; i < elf32_hdr->e_phnum; ++i)
	{
		Elf32_Phdr* cur_phdr = &(pheaders[i]);	

		if (cur_phdr->p_type == PT_LOAD)
		{
			// This is a loadable section
			if (cur_phdr->p_align > 1)
				_kpanic("ELF", "ELF loader doesn't support aligned program segments\n", 0);

			// Map these pages into memory!
			void* start_address = (void *)cur_phdr->p_vaddr;
			void* end_address   = (void *)(start_address + cur_phdr->p_memsz);
			for (; start_address < end_address; start_address += PAGE_SIZE)
			{
				__virt_map_page(__phys_get_free_4k(), start_address, PG_READ_WRITE | PG_USER);
			}

			// Lets zero it out, we only need to zero the remaining bytes, p_filesz
			// may be zero for data sections, in this case the memory should be zeroed
			_kmemclr((void *)(cur_phdr->p_vaddr + (cur_phdr->p_memsz - cur_phdr->p_filesz)), 
					cur_phdr->p_memsz - cur_phdr->p_filesz);

			// Now we have to read it in from the file
			if (cur_phdr->p_filesz > 0)
			{
				ext2_status = ext2_raw_read(bikeshed_ramdisk_context, file_name, (void *)cur_phdr->p_vaddr, 
						&bytes_read, cur_phdr->p_offset, cur_phdr->p_filesz);

				if (ext2_status != EXT2_READ_SUCCESS)
				{
					// TODO - cleanup if error
					_kpanic("ELF", "failed to read program section\n", 0);
				}
			}
		}
	}

	// Setup the PCB information

	// Allocate a stack and map some pages for it
#define NEW_STACK_LOCATION 0x2000000
#define NEW_STACK_SIZE 0x4000 /* 16 KiB */
	pcb->stack = (Uint32 *)NEW_STACK_LOCATION;
	void* stack_start = (void *)pcb->stack;
	void* stack_end   = (void *)pcb->stack + NEW_STACK_SIZE;
	for (; stack_start < stack_end; stack_start += PAGE_SIZE)
	{
		__virt_map_page(__phys_get_free_4k(), stack_start, PG_READ_WRITE | PG_USER);
	}
	_kmemclr((void *)NEW_STACK_LOCATION, NEW_STACK_SIZE);

	// TODO - We eventually want to setup a kernel stack so we can have RING 3->RING 0 access

	// Throw exit as the return address as a safe guard
	Uint32*  ptr = (Uint32 *)(NEW_STACK_LOCATION);//((Uint32 *)(pcb->stack + 1)) - 2;
	*ptr = (Uint32) exit;

	// Setup the context
	Context* context = ((Context *) ptr) - 1;
	pcb->context = context;
	_kmemclr(context, sizeof(Context));

	context->esp = NEW_STACK_LOCATION;
	context->ebp = NEW_STACK_LOCATION;
	context->cs = GDT_CODE;
	context->ss = GDT_STACK;
	context->ds = GDT_DATA;
	context->es = GDT_DATA;
	context->fs = GDT_DATA;
	context->gs = GDT_DATA;

 	// Entry point
	context->eip = elf32_hdr->e_entry;

	// Setup the rest of the PCB
	pcb->context->eflags = DEFAULT_EFLAGS;

	__kfree(pheaders);
	__kfree(elf32_hdr);
	return SUCCESS;
}
