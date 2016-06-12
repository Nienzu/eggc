	push	0
	pop	x
L000:
	push	x
	push	3
	compLT
	jz	L001
	push	x
	print
	push	x
	push	1
	add
	pop	x
	jmp	L000
L001:
