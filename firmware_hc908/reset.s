	.module reset
	.optsdcc -mhc08
	
	.area	_VECTORS (ABS)

	.org	0xfff0
	;.dw	_isrKey
	;.dw	_isrTIMEROverflow
	;.dw	_isrTIMERChannel1
	;.dw	_isrTIMERChannel0
	;.dw	_isrIRQ1
	;.dw	_isrUSB
	;.dw	_isrSWI

	.org	0xfffe
	.dw	_nomain
