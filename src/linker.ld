OUTPUT_FORMAT (elf32-i386)
OUTPUT_ARCH (i386)
ENTRY (_start)

SECTIONS {
	virt = 0xC0100000;
	phys = 0x00100000;

	.text virt : AT(phys) {
		code = .;
		*(.text)
		. = ALIGN(4096);
	}

	.rodata ALIGN(0x1000) : AT(phys + (rodata - code)) {
		rodata = .;
		*(.rodata)
		. = ALIGN(4096);
	}

	.data ALIGN(0x1000) : AT(phys + (data - code)) {
		data = .;
		*(.data)
		. = ALIGN(4096);
	}

	.bss ALIGN(0x1000) : AT(phys + (bss - code)) {
		bss = .;
		__bss_start = .;
		sbss = .;
		*(COMMON)
		*(.bss)
		ebss = .;
		_end = .;
		KERNEL_END = .;
	}
}
