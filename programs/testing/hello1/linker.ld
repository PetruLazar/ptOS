SECTIONS {
	/DISCARD/ : {
		*(.comment)
	}
	.text : {
		*(.text)
	}
	/* /DISCARD/ : { */
	.data ALIGN(0x1000) : {
		*(.data)
		*(.rodata)
		*(*)
	}
}