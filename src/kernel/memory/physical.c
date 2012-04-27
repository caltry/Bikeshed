#include "physical.h"
#include "serial.h"

#include "../lib/klib.h"

#include "c_io.h"

Uint32 *__phys_bitmap_4k;
Uint32 __phys_bitmap_4k_size_bytes;
Uint32 __phys_bitmap_4k_elements;

Uint32 KERNEL_SIZE = 0;

#define BITS_IN_INTEGER (sizeof(Uint32) * 8)

void __phys_initialize_bitmap()
{
	// Figure out how much physical memory we have	
	// Memory size in bytes / 4096 byte pages / sizeof(Uint32)*8 bits per index
	__phys_bitmap_4k_size_bytes = PHYSICAL_MEM_SIZE / PAGE_SIZE / BITS_IN_INTEGER;
	__phys_bitmap_4k_elements = PHYSICAL_MEM_SIZE / PAGE_SIZE / BITS_IN_INTEGER / sizeof(Uint32);

	// Find the location of the bitmap, make it page aligned, place it right
	// after the end of the kernel
	// We need to skip the boot page
	__phys_bitmap_4k = (Uint32*)((((Uint32)&KERNEL_END) & 0xFFFFF000) + 0x00001000);

	serial_printf("Kernel end: %x\n", (Uint32)&KERNEL_END);
	// Set the size of the kernel, include the memory Bitmap
	KERNEL_SIZE = ((Uint32)__phys_bitmap_4k + (Uint32)__phys_bitmap_4k_size_bytes) - KERNEL_LINK_ADDR;
	serial_printf("Kernel size: %d\n", (Uint32)KERNEL_SIZE);
	serial_printf("Bitmap size: %d\n", (Uint32)__phys_bitmap_4k_size_bytes);
	serial_printf("Bitmap elem: %d\n", (Uint32)__phys_bitmap_4k_elements);
	serial_printf("Bitmap location: %x\n", (Uint32)__phys_bitmap_4k);
	serial_printf("Memory size: %d\n", (Uint32)PHYSICAL_MEM_SIZE);
	serial_printf("Kernel adjusted end: %x\n", (Uint32)(KERNEL_LINK_ADDR + KERNEL_SIZE));

	// Fill the entire bitmap with 1's and then only mark free what we can
	Uint32 i = 0;
	for (; i < __phys_bitmap_4k_elements; ++i)
	{
		__phys_bitmap_4k[i] = 0xFFFFFFFF;
	}
	serial_printf("Free pages: %d\n", __phys_get_free_page_count());

	// TODO, for now just mark everything after the kernel as empty
	// because the kernel is loaded at the 1MB limit, so mark everything
	// after as usable/allocatable
	i = (KERNEL_LOAD_ADDR + KERNEL_SIZE) / PAGE_SIZE / BITS_IN_INTEGER;

	// i = 9.282
	// Grab those pages that might be free before the next index
	serial_printf("I BEFORE ALLOCATING: %d\n", i);
	++i;
	for (; i < __phys_bitmap_4k_elements; ++i)
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
	for (; i < __phys_bitmap_4k_elements; ++i)
	{
		Uint32 val = 0x1;
		Uint32 shift = 0;
		for (; shift < BITS_IN_INTEGER; ++shift)
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
	__phys_bitmap_4k[index] |= 1 << (offset % BITS_IN_INTEGER);
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
	__phys_bitmap_4k[index] &= ~(1 << (offset % BITS_IN_INTEGER));
}

void* __phys_get_free_4k()
{
	//serial_printf("Getting page, size: %d\n", __phys_bitmap_4k_size);
	//serial_printf("Free pages: %d\n", __phys_get_free_page_count());
	// Skip 32-bits at a time
	static Uint32 i = 0;
	Uint32 i_start = i;
	while (__phys_bitmap_4k[i] == 0xFFFFFFFF)
	{ 
		i = (i + 1) % __phys_bitmap_4k_elements; 
		if (i == i_start)
		{
			break;
		}
	}

	serial_printf("I after loop: %d\n", i);
	// We ran out of physical memory!
	if (__phys_bitmap_4k[i] == 0xFFFFFFFF)
	{
		serial_string("No more free pages!\n");
		for (;;) { asm("hlt"); }
	}

	// A bit has to be free because this section is != 0xFFFFFFFF
	Uint32 bit = 0x80000000, offset = 0;
	while ((__phys_bitmap_4k[i] & bit) != 0 && bit != 0)
	{
		bit >>= 1;
		++offset; // Used in address calcUint32ation
	}
	serial_printf("Offset: %d\n", offset);

	if (bit == 0) { _kpanic("Physical Memory", "Impossible Condition\n", 0); }

	__phys_bitmap_4k[i] |= bit;

	serial_printf("Phys allocating: %x\n", (i * PAGE_SIZE * BITS_IN_INTEGER) + (offset * PAGE_SIZE));
	return (void *)((i * PAGE_SIZE * BITS_IN_INTEGER) + (offset * PAGE_SIZE));
}

