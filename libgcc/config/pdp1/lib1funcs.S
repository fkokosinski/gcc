	# TODO: this shouldn't be defined here; remove it
	.global __main
__main:
	dac 0130
	jmp.i 0130

	.global __subqi3
__subqi3:
	dac.i 0131

	lac 0100
	sub 0101
	dac 0101

	lac.i 0131
	dac 0130
	jmp.i 0130

	.global __negqi2
__negqi2:
	dac.i 0131

	dzm 0101
	lac 0100
	sza
	jmp L0__negqi2
	idx 0101
L0__negqi2:
	lac.i 0131
	dac 0130
	jmp.i 0130

	.global __lshrqi3
__lshrqi3:
	dac.i 0131

	# TODO: this assumes shift == 1
	lac 0100
	ral 1
	and __lshrqi3_mask
	dac 0101

	lac.i 0131
	dac 0130
	jmp.i 0130

.data
__lshrqi3_mask:
	.byte 0777776
