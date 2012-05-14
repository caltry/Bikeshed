#include "elf.h"
#include "defs.h"
#include "fs/ext2/read_ext2.h"
#include "memory/kmalloc.h"
#include "memory/physical.h"
#include "memory/paging.h"
#include "lib/klib.h"
//#include "pcbs.h"


Status _elf_load_from_file(Pcb* pcb, const char* file_name)
{
	if (pcb == NULL || file_name == NULL) 
	{
		return BAD_PARAM;
	}

	// Try to open the file
	Elf32_Ehdr* elf32_hdr = (Elf32_Ehdr *)__kmalloc(sizeof(Elf32_Ehdr));

	Uint8 bytes_read = 0;
	ext2_read_status ext2_status =
		ext2_raw_read(file_name, elf32_hdr, &bytes_read, 0, sizeof(Elf32_Ehdr));

	if (ext2_status != EXT2_READ_SUCCESS /* Couldn't read the file */
		   || bytes_read < sizeof(Elf32_Ehdr) /* Clearly not an ELF file */
		   || elf32_hdr.e_magic != ELF_MAGIC_NUM /* Need the magic number! */
		   || elf32_hdr.e_type != ET_EXEC /* Don't support relocatable or dynamic files yet */
		   || elf32_hdr.e_machine != EM_386 /* Make sure it's for our architecture */
		   || elf32_hdr.e_entry != 0x0 /* Need an entry point */
		   || elf32_hdr.e_version != EV_CURRENT /* We don't support extensions right now */
		   || elf32_hdr.e_phoff == 0 /* If there are no program headers, what do we load? */
		   || elf32_hdr.e_phnum == 0) /* ... */
		// || elf32_hdr.e_ehsize != sizeof(Elf32_Ehdr)) /* The header size should match our struct */
	{
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

	ext2_status =
		ext2_raw_read(file_name, pheaders, &bytes_read, elf32_hdr->e_phoff, pheader_tbl_size);

	if (ext2_status != EXT2_READ_SUCCESS
			|| bytes_read < pheader_tbl_size)
	{
		__kfree(pheaders);
		__kfree(elf32_hdr);
		return BAD_PARAM;
	}

	// We need to create the new page directory for this new process
		

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
				__virt_map_page(__phys_get_free_4k(), start_address, READ_WRITE | USER)	
			}

			// Lets zero it out, we only need to zero the remaining bytes, p_filesz
			// may be zero for data sections, in this case the memory should be zeroed
			_kmemclr((void *)(cur_phdr->p_vaddr + (cur_phdr->p_memsz - cur_phdr->p_filesz)), 
					cur_phdr->p_memsz - cur_phdr->p_filesz);

			// Now we have to read it in from the file
			if (cur_phdr->p_filesz > 0)
			{
				ext2_status = ext2_raw_read(file_name, cur_phdr->p_vaddr, &bytes_read, 
						cur_phdr->p_offset, cur_phdr->filesz);
			}
		}
	}

	// Restore the old page directory
	

	// Setup the PCB information
	pcb->stack = 0;// TODO Allocate a stack
	pcb->page_directory = 0; // TODO assign the new page directory
	pcb->pid = 0;
	pcb->ppid = 0; // Assign the ppid

	pcb->context->eip = elf32_hdr->e_entry; // Entry point

	pcb->context->cs = GDT_CODE;
	pcb->context->ss = GDT_STACK;
	pcb->context->ds = GDT_DATA;
	pcb->context->es = GDT_DATA;
	pcb->context->fs = GDT_DATA;
	pcb->context->gs = GDT_DATA;

	pcb->context->eflags = DEFAULT_EFLAGS;

	__kfree(pheaders);
	__kfree(elf32_hdr);
	return BAD_PARAM; /* Not implemented yet */
}
