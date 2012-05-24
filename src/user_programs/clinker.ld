OUTPUT_FORMAT ("elf32-i386")
OUTPUT_ARCH (i386)
ENTRY (_start)

SECTIONS {

	. = 0x8000000;

	.text : {
		*(.text .text.* .gnu.linkonce.t.*)
	}

	.rodata : {
		*(.rodata .rodata.* .gnu.linkonce.r.*)
	}

	.data : {
		*(.data)
	}

	.bss : {
		*(.bss)
	}

	/DISCARD/ : {
		*(.eh_frame .note.GNU-stack .comment .note.*)
	}

}
