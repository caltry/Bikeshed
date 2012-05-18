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
#include "serial.h"

static Uint32 strlen(const char* str)
{
	Uint32 length = 0;
	while (*str != 0)
	{
		++length;
		++str;
	}

	return length;
}

static int strcmp(const char* s1, const char* s2)
{
	while (*s1 != 0 && *s2 != 0)
	{
		if (*s1 != *s2) return 0;
		++s1; ++s2;
	}

	return 1;
}

Status _elf_load_from_file(Pcb* pcb, const char* file_name)
{
	// Need to copy the file_name into kernel land...because we're killing userland!
	const char* temp = file_name;
	file_name = (const char *)__kmalloc(strlen(temp) + 1);
	_kmemcpy((void *)file_name, (void *)temp, strlen(temp)+1); // Copy the null terminator as well

	serial_printf("Elf: attempting to open: %s\n", file_name);
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
		   || elf32_hdr->e_entry == 0x0 /* Need an entry point */
		   || elf32_hdr->e_version != EV_CURRENT /* We don't support extensions right now */
		   || elf32_hdr->e_phoff == 0 /* If there are no program headers, what do we load? */
		   || elf32_hdr->e_phnum == 0) /* ... */
		// || elf32_hdr->e_ehsize != sizeof(Elf32_Ehdr)) /* The header size should match our struct */
	{
		if (ext2_status != EXT2_READ_SUCCESS) { serial_printf("RETURN VALUE: %x\n", ext2_status); _kpanic("ELF", "Failed to open file successfully\n", 0); }
		if (bytes_read < sizeof(Elf32_Ehdr)) _kpanic("ELF", "Read too small of a file!\n", 0);
		if (elf32_hdr->e_magic != ELF_MAGIC_NUM) _kpanic("ELF", "Bad magic number!\n", 0);
		if (elf32_hdr->e_type != ET_EXEC) _kpanic("ELF", "Not an executable ELF!\n", 0);
		if (elf32_hdr->e_machine != EM_386) _kpanic("ELF", "Not a i386 ELF!\n", 0);
		if (elf32_hdr->e_entry == 0x0) _kpanic("ELF", "Bad entry point!\n", 0);
		if (elf32_hdr->e_version != EV_CURRENT) _kpanic("ELF", "Don't support non-current versions!\n", 0);
		if (elf32_hdr->e_phoff == 0) _kpanic("ELF", "No program headers found!\n", 0);
		if (elf32_hdr->e_phnum == 0) _kpanic("ELF", "Zero program headers!\n", 0);

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

	serial_printf("ELF: resetting page directory\n");
	// Cleanup the old processes page directory, we're replacing everything
	__virt_switch_page_directory(pcb->page_directory);
	__virt_reset_page_directory(pcb->page_directory);

	serial_printf("ELF: About to read the program sections\n");
	/* We need to load all of the program sections now */
	for (Int32 i = 0; i < elf32_hdr->e_phnum; ++i)
	{
		Elf32_Phdr* cur_phdr = &(pheaders[i]);	

		if (cur_phdr->p_type == PT_LOAD)
		{
			if (cur_phdr->p_vaddr >= KERNEL_LINK_ADDR || cur_phdr->p_vaddr < 0x100000)
			{
				_kpanic("ELF", "An ELF with bad addresses loaded", 0);
			}

			serial_printf("\tELF: loading program section: %d at %x size: %x\n", i, cur_phdr->p_vaddr, cur_phdr->p_memsz);
			if (cur_phdr->p_memsz == 0)
			{
				serial_printf("\tELF: empty section, skipping\n");
				continue;
			}
			// This is a loadable section
			//if (cur_phdr->p_align > 1)
			//	_kpanic("ELF", "ELF loader doesn't support aligned program segments\n", 0);

			// Map these pages into memory!
			void* start_address = (void *)cur_phdr->p_vaddr;
			void* end_address   = (void *)(start_address + cur_phdr->p_memsz);
			for (; start_address < end_address; start_address += PAGE_SIZE)
			{
				Uint32 flags = PG_USER;
				if ((cur_phdr->p_flags & PF_WRITE) > 0)
				{
					flags |= PG_READ_WRITE;	
				}

				serial_printf("Checking address: %x\n", __virt_get_phys_addr(start_address));
				serial_printf("Start address: %x\n", start_address);
				if (__virt_get_phys_addr(start_address) == (void *)0xFFFFFFFF)
				{
					serial_printf("ELF: Mapping page: %x - flags: %x\n", start_address, flags);
					__virt_map_page(__phys_get_free_4k(), start_address, flags);
					serial_printf("ELF: Done mapping page\n");
				} else {
					serial_printf("Address: %x already mapped\n", start_address);
				}
			}

			serial_printf("ELF: about to memcpy program section: %x of size %d\n", cur_phdr->p_vaddr, cur_phdr->p_memsz);
			// Lets zero it out, we only need to zero the remaining bytes, p_filesz
			// may be zero for data sections, in this case the memory should be zeroed
			//_kmemclr((void *)(cur_phdr->p_vaddr + (cur_phdr->p_memsz - cur_phdr->p_filesz)), 
			//		cur_phdr->p_memsz - cur_phdr->p_filesz);
			_kmemclr((void *)cur_phdr->p_vaddr, cur_phdr->p_memsz);

			serial_printf("ELF: done memory copying: %s\n", file_name);

			// Now we have to read it in from the file
			if (cur_phdr->p_filesz > 0)
			{
				serial_printf("ELF: about to read from disk: %s - copy to: %x - size: %d\n",
						file_name, cur_phdr->p_vaddr, cur_phdr->p_filesz);
				ext2_status = ext2_raw_read(bikeshed_ramdisk_context, file_name, (void *)cur_phdr->p_vaddr, 
						&bytes_read, cur_phdr->p_offset, cur_phdr->p_filesz);

				if (ext2_status != EXT2_READ_SUCCESS)
				{
					// TODO - cleanup if error
					_kpanic("ELF", "failed to read program section\n", 0);
				}
			}
		} else {
			serial_printf("\tELF: Non-loadable section: %d at %x size: %x type: %d\n", i, cur_phdr->p_vaddr, cur_phdr->p_memsz, cur_phdr->p_type);
		}
	/*if (strcmp("/welcome", file_name))
	{
		serial_printf("File name: %s\n", file_name);
		serial_printf("Entry: %x\n", elf32_hdr->e_entry);
		asm volatile("cli");
		asm volatile("hlt");
	}
	*/
	
	}

	// Setup the PCB information

	// Allocate a stack and map some pages for it
#define NEW_STACK_LOCATION 0x2000000
#define NEW_STACK_SIZE 0x4000 /* 16 KiB */
	serial_printf("ELF: Allocating stack\n");
	void* stack_start = (void *)NEW_STACK_LOCATION;
	void* stack_end   = (void *)(NEW_STACK_LOCATION + NEW_STACK_SIZE);
	for (; stack_start < stack_end; stack_start += PAGE_SIZE)
	{
		__virt_map_page(__phys_get_free_4k(), stack_start, PG_READ_WRITE | PG_USER);
	}
	_kmemclr((void *)NEW_STACK_LOCATION, NEW_STACK_SIZE);

	// TODO - We eventually want to setup a kernel stack so we can have RING 3->RING 0 access

	// Throw exit as the return address as a safe guard
//	Uint32*  ptr = (Uint32 *)(NEW_STACK_LOCATION+NEW_STACK_SIZE-0x1000);//((Uint32 *)(pcb->stack + 1)) - 2;
	//*ptr = (Uint32) exit;
	serial_printf("ELF: setting up context\n");
	// Setup the context
	Context* context = ((Context *)(NEW_STACK_LOCATION+NEW_STACK_SIZE-4)) - 1;
	serial_printf("Context location: %x\n", context);
	pcb->context = context;

	context->esp = (Uint32)(((Uint32 *)context) - 1);
	pcb->stack = (Uint32 *)context->esp;
	context->ebp = 0x2004000-4;//(NEW_STACK_LOCATION+STACK_SIZE);
	context->cs = GDT_CODE;
	context->ss = GDT_STACK;
	context->ds = GDT_DATA;
	context->es = GDT_DATA;
	context->fs = GDT_DATA;
	context->gs = GDT_DATA;

	serial_printf("ELF: setting entry point: %x\n", elf32_hdr->e_entry);
 	// Entry point
	context->eip = elf32_hdr->e_entry;

	// Setup the rest of the PCB
	pcb->context->eflags = DEFAULT_EFLAGS;

	serial_printf("ELF: about to return\n");
	__kfree(pheaders);
	__kfree(elf32_hdr);
	__kfree((void *)file_name);
	return SUCCESS;
}
