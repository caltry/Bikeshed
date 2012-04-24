ENTRY (_start)

SECTIONS {

	.text : { 
		*(".realmode")
	}

	/DISCARD/ : {
		*(.text)
		*(.data)
		*(.rodata)
		*(.bss)
		*(COMMON)
		*(.eh_frame)
		*(.comment)
	}

}
