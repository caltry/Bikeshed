#include "physical.h"
#include "serial.h"

#include "c_io.h"

typedef long unsigned int ul;

Uint32 *__phys_bitmap_4k;
Uint32 __phys_bitmap_4k_size;

Uint32 KERNEL_SIZE = 0;

void __phys_initialize_bitmap()
{
	// Figure out how much physical memory we have	
	// Memory size in bytes / 4096 byte pages / sizeof(Uint32)*8 bits per index
	__phys_bitmap_4k_size = PHYSICAL_MEM_SIZE / 4096 / (sizeof(Uint32)*8);

	// Find the location of the bitmap, make it page aligned, place it right
	// after the end of the kernel
	// We need to skip the boot page
	__phys_bitmap_4k = (Uint32*)((((Uint32)&KERNEL_END) & 0xFFFFF000) + 0x00001000);

	serial_printf("Kernel end: %x\n", (ul)&KERNEL_END);
	// Set the size of the kernel, include the memory Bitmap
	KERNEL_SIZE = ((Uint32)__phys_bitmap_4k + (Uint32)__phys_bitmap_4k_size) - KERNEL_LINK_ADDR;
	serial_printf("Kernel size: %d\n", (ul)KERNEL_SIZE);
	serial_printf("Bitmap size: %d\n", (ul)__phys_bitmap_4k_size);
	serial_printf("Bitmap location: %x\n", (ul)__phys_bitmap_4k);
	serial_printf("Memory size: %d\n", (ul)PHYSICAL_MEM_SIZE);
	serial_printf("Kernel adjusted end: %x\n", (ul)(KERNEL_LINK_ADDR + KERNEL_SIZE));

	// Fill the entire bitmap with 1's and then only mark free what we can
	Uint32 i = 0;
	for (; i < (__phys_bitmap_4k_size / sizeof(Uint32)); ++i)
	{
		__phys_bitmap_4k[i] = 0xFFFFFFFF;
	}
	serial_printf("Free pages: %d\n", __phys_get_free_page_count());

	// TODO, for now just mark everything after the kernel as empty
	// because the kernel is loaded at the 1MB limit, so mark everything
	// after as usable/allocatable
	i = (KERNEL_LOAD_ADDR + KERNEL_SIZE) / 4096 / 32;

	// i = 9.282
	// Grab those pages that might be free before the next index
	serial_printf("I BEFORE ALLOCATING: %d\n", i);
	++i;
	for (; i < (__phys_bitmap_4k_size / sizeof(Uint32)); ++i)
	{
		__phys_bitmap_4k[i] = 0;
		serial_printf("i: %d ", i);
	}

	c_puts("Physical Memory Bitmap Initialized\n");
	serial_printf("Free pages: %d\n", __phys_get_free_page_count());
}

Uint32 __phys_get_free_page_count()
{
	Uint32 i = 0;
	Uint32 sum = 0;
	for (; i < (__phys_bitmap_4k_size / sizeof(Uint32)); ++i)
	{
		Uint32 val = 0x1;
		Uint32 shift = 0;
		for (; shift < sizeof(Uint32)*8; ++shift)
		{
			sum += (__phys_bitmap_4k[i] & val) == 0;
			val <<= 1;
		}
	}

	return sum;
}

void __phys_set_bit(void* address)
{
	// TODO check bounds
	// address / 4096;
	Uint32 offset = (Uint32)address >> 12;
	Uint32 index = offset / sizeof(Uint32);
	__phys_bitmap_4k[index] |= 1 << (offset % (sizeof(Int32)*8));
}

Uint32 __phys_check_bit(void* address)
{
	// TODO check bounds
	Uint32 offset = (Uint32)address >> 12;
	Uint32 index = offset / sizeof(Uint32);
	return __phys_bitmap_4k[index];
}

void __phys_unset_bit(void* address)
{
	// TODO check bounds
	Uint32 offset = (Uint32)address >> 12;
	Uint32 index = offset / sizeof(Uint32);
	__phys_bitmap_4k[index] &= ~(1 << (offset % (sizeof(Int32)*8)));
}

void* __phys_get_free_4k()
{
	//serial_printf("Getting page, size: %d\n", __phys_bitmap_4k_size);
	//serial_printf("Free pages: %d\n", __phys_get_free_page_count());
	// Skip 32-bits at a time
	Uint32 i = 0;
	while (__phys_bitmap_4k[i] == 0xFFFFFFFF && i < (__phys_bitmap_4k_size / sizeof(Uint32))) { ++i; }

	serial_printf("I after loop: %d\n", i);
	// We ran out of physical memory!
	if (i >= (__phys_bitmap_4k_size / sizeof(Uint32)))
	{
		serial_string("No more free pages!\n");
		for (;;) { asm("hlt"); }
	}

	// A bit has to be free because this section is != 0xFFFFFFFF
	Uint32 bit = 0x80000000, offset = 0;
	while ((__phys_bitmap_4k[i] & bit) != 0 && bit != 0)
	{
		bit >>= 1;
		++offset; // Used in address calculation
	}
	serial_printf("Offset: %d\n", offset);

	if (bit == 0) { _kpanic("Physical Memory", "Impossible Condition\n", 0); }

	__phys_bitmap_4k[i] |= bit;

	// (i * sizeof(Uint32) + offset) * 4096
	serial_printf("Phys allocating: %x\n", (i * 4096 * 32) + (offset * 4096));
	return (void *)((i * 4096 * 32) + (offset * 4096));
}

