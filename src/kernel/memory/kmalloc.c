#include "kmalloc.h"
#include "../serial.h"
#include "paging.h"
#include "physical.h"

#include "../../ulib/c_io.h"
#include "../lib/klib.h"

typedef struct LinkedNode
{
	uint32 size; // Size must be first!
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

// 256 MB of Kernel Heap Space
#define HEAP_START_LOCATION 0xD0000000
#define HEAP_MAX_LOCATION   0xE0000000
#define HEAP_INITIAL_PAGES 2

void __kmem_init_kmalloc()
{
	serial_string("Kmalloc intializing\n");
	kernel_heap.start_address = (void*)HEAP_START_LOCATION;
	kernel_heap.max_address = (void*)HEAP_MAX_LOCATION;

	void* start_address = (void*)HEAP_START_LOCATION;
	int32 i = 0;
	for (; i < HEAP_INITIAL_PAGES; ++i)
	{
		serial_string("Kmalloc requesting page\n");
		__virt_map_page(__phys_get_free_4k(), start_address, READ_WRITE | PRESENT);
		start_address += 4096;
	}

	kernel_heap.end_address = start_address;
	kernel_heap.start_node = (linked_node_t*)HEAP_START_LOCATION;

	kernel_heap.start_node->size = HEAP_INITIAL_PAGES * 4096 - HEADER_SIZE;
	kernel_heap.start_node->next = 0;
	kernel_heap.start_node->prev = 0;

	serial_printf("Start address: %X\n", (Uint32)kernel_heap.start_address);
	serial_printf("End address:   %X\n", (Uint32)kernel_heap.end_address);
	serial_printf("Max address:   %X\n", (Uint32)kernel_heap.max_address);
	serial_printf("Node Start:    %X\n", (Uint32)kernel_heap.start_node);
	serial_printf("Node next:     %X\n", (Uint32)kernel_heap.start_node->next);
	serial_printf("Node prev:     %X\n", (Uint32)kernel_heap.start_node->prev);
	serial_printf("Node size:     %d\n", (Uint32)sizeof(linked_node_t));

	c_puts("Kmalloc Initialized\n");
}

void* __kmalloc(uint32 size)
{
	serial_string("----Allocating----\n");
	serial_printf("Input size: %d\n", (Uint32)size);
	// When we add it back to the free list, we need at least
	// this many bytes
	if (size < sizeof(linked_node_t) - HEADER_SIZE)
	{
		size = sizeof(linked_node_t) - HEADER_SIZE;
		serial_printf("New size: %d\n", (Uint32)size);
	}

	// Make it word aligned
	if (size % 4 != 0)
	{
		serial_printf("Aligning size: %d\n", (Uint32)(size % 4));
		size += 4 - (size % 4);
		serial_printf("Aligned size: %d\n", (Uint32)size);
	}

	// Check the nodes for the correct size
	linked_node_t* current_node = kernel_heap.start_node;
	while (current_node->next != 0 && current_node->size <= size)
	{
		current_node = current_node->next;
	}

	serial_printf("Node found? %X\n", (Uint32)current_node);

	// We couldn't find a node of sufficient size
	// so ask for a page
	if (current_node->next == 0 && current_node->size < size)
	{
		uint32 num_pages = (size - current_node->size) / 4096 + 1;
		serial_printf("No node found, allocating %d pages\n", (Uint32)num_pages);
		uint32 i = 0;
		for (; i < num_pages; ++i)
		{
			if (kernel_heap.end_address > kernel_heap.max_address)
			{
				serial_string("Kernel heap size limit reached!\n");
				for (;;) { asm("hlt"); }
			}
			
			// Add a new page to the end of our heap
			__virt_map_page(__phys_get_free_4k(), kernel_heap.end_address, READ_WRITE | PRESENT);
			kernel_heap.end_address += 4096;
		}

		// Expand the node
		current_node->size = (Uint32)kernel_heap.end_address - (uint32)current_node - HEADER_SIZE;

		serial_printf("Current  node addr: %X\n", (Uint32)current_node);
	}

	serial_printf("Current  node size: %d\n", (Uint32)current_node->size);

	// Need to setup the next node
	if (((uint32)current_node + HEADER_SIZE + size + sizeof(linked_node_t)) >= (Uint32)kernel_heap.end_address)
	{
		// We need more space, allocate 1 more page
		__virt_map_page(__phys_get_free_4k(), kernel_heap.end_address, READ_WRITE | PRESENT);
		kernel_heap.end_address += 4096;

		if (kernel_heap.end_address > kernel_heap.max_address)
		{
				serial_string("Kernel heap size limit reached!\n");
				for (;;) { asm("hlt"); }
		}
	}

	// Allocate the next node
	linked_node_t* next_node = (linked_node_t*)((uint32)current_node + size + HEADER_SIZE);

	if (current_node == kernel_heap.start_node)
	{
		serial_string("Next node is head node\n");
		kernel_heap.start_node = next_node;
		next_node->prev = 0;
		next_node->next = current_node->next;
	} else {
		serial_string("Next node is a middle node\n");
		next_node->next = current_node->next;
		next_node->prev = current_node->prev;
		current_node->prev->next = next_node;

		if (current_node->next != 0)
		{
			current_node->next->prev = next_node;
		}
	}

	// Setup next_node's size
	if (current_node->next == 0)
	{
		next_node->size = (Uint32)kernel_heap.end_address - (uint32)next_node - HEADER_SIZE;
	} else {
		next_node->size = (uint32)current_node->next - (uint32)next_node - HEADER_SIZE;
	}

	serial_printf("Next node size: %d\n", (Uint32)next_node->size);
	serial_printf("Next node addr: %x\n", (Uint32)next_node);

	serial_printf("Returning address: %X\n", (uint32)current_node + HEADER_SIZE);

	serial_string("----End Allocating----\n");
	current_node->size = size;
	// Give them the address
	return (void *)((uint32)current_node + HEADER_SIZE);
}

/*
void* realloc(void* address, uint32 new_size)
{
}
*/

/*
void* kcalloc(void)
{
} */ 
/* For debugging */
int list_size(void)
{
	int size = 0;
	linked_node_t* node = kernel_heap.start_node;
	while (node != 0)
	{
		node = node->next;
		++size;
	}

	return size;
}

void __kmem_kmalloc_tests()
{
	serial_string(".---- Start of kmalloc() tests ----.\n");
	serial_string("|If all is working correctly there |\n");
	serial_string("|should be an 'All Tests Passed'   |\n");
	serial_string("|message                           |\n");
	serial_string("'----------------------------------'\n");
	// This function makes sure the __kmalloc routines are working correctly
	
	// Test 1 - Assumes that nothing has been allocated yet, calls __kmalloc() and then __kfree()
	// In free it's the code path:
	//  if (free_node < current_node)
	//      if ((uint32)free_node + free_node->size + HEADER_SIZE) == current_node)
	serial_string("========Test 1========\n");
	void* ptr = __kmalloc(10);
	if ((Uint32)ptr != 0xC0000004) { _kpanic("Kmalloc", "Bad address returned from __kmalloc()\n", 0); }
	if ((Uint32)kernel_heap.start_node != 0xC0000010) { _kpanic("Kmalloc", "Wrong address for next node\n", 0); }
	if (kernel_heap.start_node->size != (8192 - 12 - 2*HEADER_SIZE)) { _kpanic("Kmalloc", "Next node has a bad size!\n", 0); }
	if (kernel_heap.start_node->next != 0) { _kpanic("Kmalloc", "Next node is not 0!\n", 0); }
	if (kernel_heap.start_node->prev != 0) { _kpanic("Kmalloc", "Prev node is not 0!\n", 0); }
	__kfree(ptr);
	if ((Uint32)kernel_heap.start_node != 0xC0000000) { _kpanic("Kmalloc", "Test 1 - __kfree() messed up\n", 0); }
	if (kernel_heap.start_node->size != (8192 -  HEADER_SIZE)) { _kpanic("Kmalloc", "Free'd node has a bad size!\n", 0); }
	if (kernel_heap.start_node->next != 0) { _kpanic("Kmalloc", "Free'd next node is not 0!\n", 0); }
	if (kernel_heap.start_node->prev != 0) { _kpanic("Kmalloc", "Free'd prev node is not 0!\n", 0); }

	// Test 2 - Allocate three things and free the first, tests the code path in free
	// if (free_node < current_node)
	//    else - Can't combine 
	serial_string("========Test 2========\n");
	ptr  = __kmalloc(10);
	void* ptr2 = __kmalloc(10);
	void* ptr3 = __kmalloc(10);

	serial_string("PTR1\n");
	__kfree(ptr);
	serial_string("PTR3\n");
	__kfree(ptr3);
	serial_string("PTR2\n");
	__kfree(ptr2);

	serial_printf("Number of Heap nodes: %d\n", (Uint32)list_size());
	
	serial_string("========Test 3========\n");
	ptr = __kmalloc(10);
	

	serial_string("---- All Tests Passed ----\n");
}

void __kfree(void* address)
{
	serial_string("----Free----\n");
	// Figure out where this node goes
	linked_node_t* free_node = (linked_node_t*)((uint32)address - HEADER_SIZE);

	serial_printf("Freeing address: %X\n", (Uint32)free_node);

	// Check to see if it's replacing the head node
	linked_node_t* current_node = kernel_heap.start_node;
	if (free_node < current_node) // They shouldn't ever be equal
	{
		serial_printf("Current node: %X\n", (Uint32)current_node);
		serial_printf("Current node size: %d\n", (Uint32)current_node->size);
		serial_printf("Free_node size   : %d\n", (Uint32)free_node->size);
		serial_string("Free_node before head node\n");
		// First check to see if they can be combined
		if (((uint32)free_node + free_node->size + HEADER_SIZE) == (Uint32)current_node)
		{
			serial_string("Can combine free_node and start_node\n");
			kernel_heap.start_node = free_node;

			// Need to replace the prev/next pointers
			free_node->size += current_node->size + HEADER_SIZE;
			free_node->next = current_node->next;
			free_node->prev = 0;
		} else {
			serial_string("Can't combine free_node and start_node\n");
			// We need to replace the head
			kernel_heap.start_node = free_node;
			free_node->next = current_node;
			free_node->prev = 0;
			current_node->prev = free_node;
		}

		serial_string("----End Free----\n");
		return;
	}

	// We aren't replacing the head, find the correct position in the list
	while (current_node->next != 0 && current_node < free_node)
	{
		current_node = current_node->next;
	}

	// Stores the node before free_node and current_node
	linked_node_t* prev_node = current_node->prev;
	linked_node_t* middle_node = free_node;
	linked_node_t* last_node = current_node;

	serial_printf("Current node: %x\n", (Uint32)current_node);
	if (free_node < current_node)
	{
		serial_string("free node before tail\n");
		current_node->prev->next = free_node;
		free_node->prev = current_node->prev;
		free_node->next = current_node;
		current_node->prev = free_node;
	} else {
		// TODO we'll never be freeing from behind the tail...
		serial_string("free node after tail\n");
		// Check if we ended up at the tail
		if (current_node->next != 0)
		{
			current_node->next->prev = free_node;
		}

		free_node->next = current_node->next;
		free_node->prev = current_node;
		current_node->next = free_node;

		// Used for combinations
		middle_node = current_node;
		last_node = free_node;
	}

	// Check for combinations
	if (((uint32)prev_node + prev_node->size + HEADER_SIZE) == (Uint32)middle_node)
	{
		serial_string("Combining prev and middle\n");
		prev_node->size += middle_node->size + HEADER_SIZE;
		prev_node->next = middle_node->next;
		if (middle_node->next == 0)
		{
			_kpanic("Kmalloc", "Next shouldn't be 0!\n", 0);
		}

		prev_node->next->prev = prev_node;

		middle_node = prev_node;
	}
	
	if (((uint32)middle_node + middle_node->size + HEADER_SIZE) == (Uint32)last_node)
	{
		serial_string("Combining middle and next\n");
		middle_node->size += last_node->size + HEADER_SIZE;
		middle_node->next = last_node->next;
		if (last_node->next != 0)
		{
			serial_string("Last node was null\n");
			middle_node->next->prev = middle_node;
		}
	}

	serial_string("----End Free----\n");
}
