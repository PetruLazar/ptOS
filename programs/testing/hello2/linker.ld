SECTIONS {
	/DISCARD/ : {
		*(.comment)
	}
	.text : {
		QUAD(entry)
		*(.text)
	}
	/* /DISCARD/ : { */
	.data ALIGN(0x1000) : {
		*(.data)
		*(.rodata)
		*(*)
	}
}