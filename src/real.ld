ENTRY (_start)

SECTIONS {

	.realmode : { 
		*(".realmode")
	}

	/DISCARD/ : {
		*(.text)
		*(.data)
		*(.rodata)
		*(.bss)
		*(COMMON)
	}

}
