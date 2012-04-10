#include "physical.h"
#include "serial.h"

#include "c_io.h"

typedef long unsigned int ul;

uint32 *__phys_bitmap_4k;
uint32 __phys_bitmap_4k_size;

uint32 KERNEL_SIZE = 0;

void __phys_initialize_bitmap()
{
	// Figure out how much physical memory we have	
	// Memory size in bytes / 4096 byte pages / sizeof(uint32)*8 bits per index
	__phys_bitmap_4k_size = PHYSICAL_MEM_SIZE / 4096 / (sizeof(uint32)*8);

	// Find the location of the bitmap, make it page aligned, place it right
	// after the end of the kernel
	// We need to skip the boot page
	__phys_bitmap_4k = (uint32*)((((uint32)&KERNEL_END) & 0xFFFFF000) + 0x00001000);

	serial_printf("Kernel end: %x\n", (ul)&KERNEL_END);
	// Set the size of the kernel, include the memory Bitmap
	// the 0x2000 is the fudge from above
	//KERNEL_SIZE = ((uint32)&KERNEL_END) - KERNEL_LINK_ADDR + __phys_bitmap_4k_size;
	KERNEL_SIZE = ((uint32)__phys_bitmap_4k + (uint32)__phys_bitmap_4k_size) - KERNEL_LINK_ADDR;
	serial_printf("Kernel size: %d\n", (ul)KERNEL_SIZE);
	serial_printf("Bitmap size: %d\n", (ul)__phys_bitmap_4k_size);
	serial_printf("Bitmap location: %x\n", (ul)__phys_bitmap_4k);
	serial_printf("Memory size: %d\n", (ul)PHYSICAL_MEM_SIZE);
	serial_printf("Kernel adjusted end: %x\n", (ul)(KERNEL_LINK_ADDR + KERNEL_SIZE));

	// Fill the entire bitmap with 1's and then only mark free what we can
	uint32 i = 0;
	for (; i < __phys_bitmap_4k_size; ++i)
	{
		__phys_bitmap_4k[i] = 0xFFFFFFFF;
	}

	// TODO, for now just mark everything after the kernel as empty
	// because the kernel is loaded at the 1MB limit, so mark everything
	// after as usable/allocatable
	i = (KERNEL_LOAD_ADDR + KERNEL_SIZE) / 4096 / sizeof(uint32);

	// i = 9.282
	// Grab those pages that might be free before the next index
	++i;
	for (; i < __phys_bitmap_4k_size; ++i)
	{
		__phys_bitmap_4k[i] = 0;
	}

	c_puts("Physical Memory Bitmap Initialized\n");
}

void __phys_set_bit(void* address)
{
	// TODO check bounds
	// address / 4096;
	uint32 offset = (uint32)address >> 12;
	uint32 index = offset / sizeof(uint32);
	__phys_bitmap_4k[index] |= 1 << (offset % (sizeof(int32)*8));
}

uint32 __phys_check_bit(void* address)
{
	// TODO check bounds
	uint32 offset = (uint32)address >> 12;
	uint32 index = offset / sizeof(uint32);
	return __phys_bitmap_4k[index];
}

void __phys_unset_bit(void* address)
{
	// TODO check bounds
	uint32 offset = (uint32)address >> 12;
	uint32 index = offset / sizeof(uint32);
	__phys_bitmap_4k[index] &= ~(1 << (offset % (sizeof(int32)*8)));
}

void* __phys_get_free_4k()
{
	// Skip 32-bits at a time
	uint32 i = 0;
	while (__phys_bitmap_4k[i] == 0xFFFFFFFF && i < __phys_bitmap_4k_size) { ++i; }

	// We ran out of physical memory!
	if (i >= __phys_bitmap_4k_size)
	{
		serial_string("No more free pages!\n");
		for (;;) { asm("hlt"); }
	}

	// A bit has to be free because this section is != 0xFFFFFFFF
	uint32 bit = 0x80000000, offset = 0;
	while ((__phys_bitmap_4k[i] & bit) != 0 && bit != 0)
	{
		bit >>= 1;
		++offset; // Used in address calculation
	}

	if (bit == 0) { serial_string("Impossible condition!\n"); for (;;) { asm("hlt"); } }

	__phys_bitmap_4k[i] |= bit;

	// (i * sizeof(uint32) + offset) * 4096
	return (void *)(((i * sizeof(uint32)) + offset) << 12);
}

