SECTIONS {
	/DISCARD/ : {
		*(.comment)
	}
	.text : {
		QUAD(main)
		*(.text)
	}
	/* /DISCARD/ : { */
	.data ALIGN(0x1000) : {
		*(.data)
		*(.rodata)
		*(*)
	}
}