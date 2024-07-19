	.file		"crti.S"

	.section	".init"
	.global	_init
	.type	_init, @function	
	#.p2align	1
_init:
	.skip 1

	.section	".fini"
	.global	_fini
	.type	_fini,@function
	#.p2align	1
_fini:
	.skip 1
