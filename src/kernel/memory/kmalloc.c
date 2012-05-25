#include "defs.h"

#include "kmalloc.h"
#include "paging.h"
#include "physical.h"

#include "serial.h"

#include "c_io.h"
#include "lib/klib.h"

#include "scheduler.h"


#define serial_string(...) 
#define serial_printf(...)

typedef struct LinkedNode
{
	Uint32 size; // Size must be first! Also the size without the HEADER
	struct LinkedNode* next;
	struct LinkedNode* prev;
} linked_node_t;

#define HEADER_SIZE 4

typedef struct Heap
{
	void* start_address;
	void* end_address;
	void* max_address;
	linked_node_t* start_node;
} heap_t;

heap_t kernel_heap;

extern Uint32* scratch_pd;

#define HEAP_INITIAL_PAGES  2

void __kmem_init_kmalloc()
{
	kernel_heap.start_address = (void *)HEAP_START_LOCATION;
	kernel_heap.max_address   = (void *)HEAP_MAX_LOCATION;

	void* start_address = kernel_heap.start_address;
	for (Uint32 i = 0; i < HEAP_INITIAL_PAGES; ++i)
	{
		__virt_map_page(__phys_get_free_4k(), start_address, PG_READ_WRITE | PG_PRESENT);
		start_address += PAGE_SIZE;
	}

	kernel_heap.end_address = start_address;
	kernel_heap.start_node = (linked_node_t *)kernel_heap.start_address;

	kernel_heap.start_node->size = HEAP_INITIAL_PAGES * PAGE_SIZE;
	kernel_heap.start_node->next = 0;
	kernel_heap.start_node->prev = 0;
}

void __kmalloc_info(void)
{
	Uint32 node_number = 1;
	linked_node_t* node = kernel_heap.start_node;

	while (node != 0)
	{
		serial_printf("Node Number: %d\n", node_number);
		serial_printf("Node addr: %x\n", node);
		serial_printf("Node size: %d\n", node->size);
		serial_printf("Node next: %x\n", node->next);
		serial_printf("Node prev: %x\n\n", node->prev);
		node = node->next;
		++node_number;
	}
}

void print_node_info(linked_node_t* node)
{
	UNUSED(node); // Cheat for when we 'remove' the print statements
	serial_printf("Node addr: %x\n", node);
	serial_printf("Node size: %d\n", node->size);
	serial_printf("Node next: %x\n", node->next);
	serial_printf("Node prev: %x\n\n", node->prev);
}

void* __kmalloc(Uint32 size)
{
	serial_printf("Kmalloc\nUnmodded size: %d\n", size);
	// In order to make sure there is enough room, we can't allocate
	// less than sizeof(linked_node_t) bytes, otherwise we won't have
	// room to store the linked list information when we add the chunk
	// of memory back to the free list
	if (size < sizeof(linked_node_t))
	{
		size = sizeof(linked_node_t);
	}

	// Make sure the size is aligned to a 4-byte boundary
	if (size % 4 != 0)
	{
		size += 4 - (size % 4);
	}

	// Add the header size to size, because we subtract that when returning the node
	size += HEADER_SIZE;

	// Find a node that will fit it
	linked_node_t* current_node = kernel_heap.start_node;
	while (current_node->next != 0 && current_node->size <= (size + sizeof(linked_node_t)))
	{
		current_node = current_node->next;
	}

	serial_string("Before loop\n");
	serial_printf("Size: %d\n", size);
	// We can shorten this to
	// if (current_node->size < size)
	if (current_node->next == 0 && current_node->size < (size + sizeof(linked_node_t)))
	{
		// We couldn't find a node large enough, ask for more space!
		// We need to allocate at least 1 page, but also include some fudge for the next
		// header which will need to come after this as we're expanding the last node
		Uint32 num_pages = (size - current_node->size + sizeof(linked_node_t)) / PAGE_SIZE + 1;

		// We need to do something special, we need a common place for all of the other
		// processes to get the new page locations from. So we'll switch to the kpage_directory
		// page directory, map the pages, and then copy them to the current process. This way
		// if we get a page fault from another process we know where to get the missing pages
		// from

		// Switch to the base page directory
		__virt_switch_page_directory(__virt_kpage_directory);

		for (Uint32 i = 0; i < num_pages; ++i)
		{
			if (kernel_heap.end_address > kernel_heap.max_address)
			{
				_kpanic("Kmalloc", "Heap has exceeded the set bounds!\n", 0);
			}

			serial_string("Alloc page\n");
			// Allocate a page to the end of the heap
			__virt_map_page(__phys_get_free_4k(), kernel_heap.end_address, PG_READ_WRITE | PG_PRESENT);
			kernel_heap.end_address += PAGE_SIZE;
		}

		// This is clumsy would have been better to have a statically allocated kernel directory
		// then this would be a simple loop...
		__virt_clear_page(scratch_pd);
		__virt_map_page(_current->page_directory, scratch_pd, PG_READ_WRITE);

		Uint32* pd = PAGE_DIR_ADDR;
		for (Uint32 i = ADDR_TO_PD_IDX(HEAP_START_LOCATION); i < ADDR_TO_PD_IDX(HEAP_MAX_LOCATION); ++i)
		{
			serial_printf("KMALLOC - INDEX: %d\n", i);
			scratch_pd[i] = pd[i];
		}
		__virt_clear_page(scratch_pd);

		// Switch back
		__virt_switch_page_directory(_current->page_directory);

		// Adjust current_node's size accordingly
		current_node->size += num_pages * PAGE_SIZE;
	}

	// Okay we've found a good node
	linked_node_t* next_node = (linked_node_t *)((Uint32)current_node + size);	
	// Setup next_node's size
	next_node->size = (Uint32)current_node->size - size;

	serial_string("Next node:\n");
	print_node_info(next_node);
	serial_string("Current node:\n");
	print_node_info(current_node);
	
	// Check if we're replacing the root node
	if (current_node == kernel_heap.start_node)
	{
		kernel_heap.start_node = next_node;
		next_node->prev = 0;
		next_node->next = current_node->next;
	} else {
		next_node->next = current_node->next;
		next_node->prev = current_node->prev;

		current_node->prev->next = next_node;
	}

	serial_string("good\n");

	// Add the previous node for both cases
	if (current_node->next != 0)
	{
		current_node->next->prev = next_node;
	}

	// Fix up the current_node's size so we know how much to free!
	current_node->size = size;

	current_node->next = (void *)0xDEADBEEF; // Sentinel pointers to see if something is wrong later
	current_node->prev = (void *)0xCAFEBABE;

	serial_string("End Kmalloc\n");
	return (void *)((Uint32)current_node + HEADER_SIZE);
}

void* __kcalloc(Uint32 size)
{
	void* address = __kmalloc(size);
	_kmemclr(address, size);

	return address;
}

void __kfree(void* address)
{
	// Do nothing if we've been given a bad value
	if (address < kernel_heap.start_address || address > kernel_heap.end_address)
	{
		serial_printf("==============KMALLOC OUT OF RANGE ADDRESS: %x\n", address);
		return;
	}

	serial_string("Kfree\n");
	linked_node_t* free_node = (linked_node_t *)((Uint32)address - HEADER_SIZE);

	serial_printf("Free node->next: %x\n", (Uint32)free_node->next);
	serial_printf("Free node->prev: %x\n", (Uint32)free_node->prev);

	if (free_node->size > ((Uint32)kernel_heap.end_address - (Uint32)kernel_heap.start_address))
	{
		serial_printf("Bad size!\n");
		serial_printf("Free node size: %d\n", free_node->size);
		serial_printf("Current: %d\n", _current->pid);
		serial_printf("Current stack: %d\n", _current->stack);
		serial_printf("Current esp: %d\n", _current->context->esp);
		_kpanic("Kmalloc", "Bad size!\n", 0);
	}

	linked_node_t* current_node = kernel_heap.start_node;	
	// Check to see if this free_node is before the current head of the free list
	if (free_node < current_node)
	{
		// Check if we can combine the free_node with the start_node	
		if (((Uint32)free_node + free_node->size) == (Uint32)current_node)
		{
			serial_string("Combining free_node with start_node\n");
			free_node->size += kernel_heap.start_node->size;
			free_node->next = kernel_heap.start_node->next;
			free_node->prev = 0;

			if (kernel_heap.start_node->next != 0)
			{
				kernel_heap.start_node->next->prev = free_node;
			}
		} else {
			// We can't combine the free_node with start_node so we have to replace it
			free_node->next = kernel_heap.start_node;
			free_node->prev = 0;
			kernel_heap.start_node->prev = free_node;
		}

		kernel_heap.start_node = free_node;
		serial_string("End Kfree\n");
		return; // We've found a place for the free_node
	}

	// Okay we still haven't found a place for the free_node
	// at this point current_node == kernel_heap.start_node
	// Loop until we find a node that is after free_node and
	// then insert free_node before that node
	while (current_node != 0 && current_node < free_node)
	{
		current_node = current_node->next;
	}

	// Okay either we fell off the list (which should be impossible)
	// or we found a node that comes after free_node
	if (current_node == 0)
	{
		// We fell off the list! This should be impossible!
		_kpanic("Kmalloc", "Fell off the free list, impossible condition!\n", 0);
	}

	// current_node is now a node that should come after free_node
	// So setup the relations			
	free_node->next = current_node;
	free_node->prev = current_node->prev;
	current_node->prev->next = free_node;	
	current_node->prev = free_node;

	// These should all point to the correct places now
	linked_node_t* prev_node   = free_node->prev;
	linked_node_t* middle_node = free_node;
	linked_node_t* last_node   = free_node->next;

	// Check if the previous node can be combined with free_node
	if (((Uint32)prev_node + prev_node->size) == (Uint32)middle_node)
	{
		// We can combine them
		prev_node->next = middle_node->next;
		middle_node->next->prev = prev_node;

		// Update prev_node's size
		prev_node->size += free_node->size;

		// Make us the new middle
		middle_node = prev_node;
	}

	// Check if the middle node can be combined with the last_node
	if (((Uint32)middle_node + middle_node->size) == (Uint32)last_node)
	{
		// We can combine them
		middle_node->next = last_node->next;
		if (last_node->next != 0)
		{
			last_node->next->prev = middle_node;
		}

		// Update middle_node's size
		middle_node->size += last_node->size;
	}

	serial_string("End Kfree\n");
}

void __kmem_kmalloc_tests(Uint32 test_size)
{
	const Uint32 amt = test_size;

	void *ptrs[amt];

	Uint32 total_allocated = 0;
	/* Run some kmalloc() tests */
	for (Uint32 i = 0; i < amt; ++i)
	{
		Uint32 size = _krand() % 8192;
		ptrs[i] = __kmalloc(size);
		total_allocated += size;
	}

	Uint32 num_freed = 0;
	while (num_freed != amt)
	{
		Uint32 index = _krand() % amt;
		if (ptrs[index] != 0)
		{
			__kfree(ptrs[index]);
			ptrs[index] = 0;
			++num_freed;
		}
	}

	serial_printf("Total amount allocated: %d\n", total_allocated);

	__kmalloc_info();

	asm volatile ("cli");
	asm volatile ("hlt");
}

void __kmem_kmalloc_tests_2(Uint32 test_size)
{
	const Uint32 amt = test_size;

	void *ptrs[amt];

	Uint32 total_allocated = 0;
	/* Run some kmalloc() tests */
	for (Uint32 i = 0; i < amt; ++i)
	{
		Uint32 size = _krand() % 8192;
		ptrs[i] = __kmalloc(size);
		total_allocated += size;
	}

	Uint32 num_freed = 0;
	while (num_freed != amt/2)
	{
		Uint32 index = _krand() % amt;
		if (ptrs[index] != 0)
		{
			__kfree(ptrs[index]);
			ptrs[index] = 0;
			++num_freed;
		}
	}

	for (Uint i = 0; i < amt; ++i)
	{
		if (ptrs[i] == 0)
		{
			Uint32 size = _krand() % 8192;
			ptrs[i] = __kmalloc(size);
			total_allocated += size;
		}
	}		

	num_freed = 0;
	while (num_freed != amt)
	{
		Uint32 index = _krand() % amt;
		if (ptrs[index] != 0)
		{
			__kfree(ptrs[index]);
			ptrs[index] = 0;
			++num_freed;
		}
	}

	serial_printf("Total amount allocated: %d\n", total_allocated);

	__kmalloc_info();

	asm volatile ("cli");
	asm volatile ("hlt");
}
