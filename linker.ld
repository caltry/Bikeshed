ENTRY (loader)

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
		sbss = .;
		*(COMMON)
		*(.bss)
		ebss = .;
		KERNEL_END = .;
	}
}
