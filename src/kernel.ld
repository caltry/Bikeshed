ENTRY (_start)

SECTIONS {
	virt = 0xC0100000;
	phys = 0x00100000;

	.realmode 0x3000 : AT(0x3000) {
		real = .;
		__start_real = ABSOLUTE(.);

		kernel/bios/realmode.o(.text)
		kernel/bios/realmode.o(.rodata)
		kernel/bios/realmode.o(.data)
		kernel/bios/realmode.o(COMMON)
		kernel/bios/realmode.o(.bss)

		__end_real = ABSOLUTE(.);
	}

	.text virt : AT(phys) {
		code = .;
		*(.text)
		*(.gnu.linkonce.t*)
		. = ALIGN(4096);
	}

	.rodata ALIGN(0x1000) : AT(phys + (rodata - code)) {
		start_ctors = .;
		*(.ctor*)
		end_ctors = .;

		start_dtors = .;
		*(.dtor*)
		end_dtors = .;

		rodata = .;
		*(.rodata)
	}

	.data ALIGN(0x1000) : AT(phys + (data - code)) {
		data = .;
		*(.data)
	}

	.bss ALIGN(0x1000) : AT(phys + (bss - code)) {
		bss = .;
		__bss_start = .;
		*(COMMON)
		*(.bss)
		*(.gnu.linkonce.b*)

		_end = .;
		KERNEL_END = .;
	}

	/DISCARD/ :
	{
		*(.comment)
		*(.eh_frame)
	}
}
