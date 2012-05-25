OUTPUT_FORMAT ("elf32-i386")
OUTPUT_ARCH (i386)
ENTRY (_start)

SECTIONS {

	/* All code/data starts at 0x8000000 */
	. = 0x8000000;

	/* Include the extra GNU sections in .text */
	.text : {
		*(.text .text.* .gnu.linkonce.t.*)
	}

	/* Include the extra GNU sections in .rodata */
	.rodata : {
		*(.rodata .rodata.* .gnu.linkonce.r.*)
	}

	/* Regular data section */
	.data : {
		*(.data)
	}

	/* Regular bss section */
	.bss : {
		*(.bss)
	}

	/* We don't care about these extra sections */
	/DISCARD/ : {
		*(.eh_frame .note.GNU-stack .comment .note.*)
	}

}
