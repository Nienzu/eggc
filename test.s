	.data
idMain_idA:	.word	0
idMain_idB:	.word	0
idMain_idResult:	.word	0
idMain_idLoop:	.word	0
idMain_idArray:	.space	40
	.text
	.globl main
main:
	li $v0, 5
	syscall
	sw $v0, idMain_idA
	li $v0, 5
	syscall
	sw $v0, idMain_idB
	li $v0, 1
	lw $a0, idMain_idA
	syscall
	li $v0, 1
	lw $a0, idMain_idB
	syscall
	lw $s0, idMain_idA
	lw $s1, idMain_idB
	add $t0, $s0, $s1
	move $s0, $t0
	lw $s1, idMain_idB
	mul $t1, $s0, $s1
	move $s0, $t1
	sw $s0, idMain_idResult

	li $v0, 1
	lw $a0, idMain_idResult
	syscall
	lw $s0, idMain_idA
	lw $s1, idMain_idB
	sub $t0, $s0, $s1
	move $s0, $t0
	lw $s1, idMain_idResult
	add $t1, $s0, $s1
	move $s0, $t1
	lw $s1, idMain_idResult
	mul $t2, $s0, $s1
	move $s0, $t2
	sw $s0, idMain_idResult

	lw $s0, idMain_idA
	lw $s1, idMain_idB
	add $t0, $s0, $s1
	move $s0, $t0
	lw $s1, idMain_idA
	sub $t1, $s0, $s1
	move $s0, $t1
	lw $s1, idMain_idB
	mul $t2, $s0, $s1
	move $s0, $t2
	div $t3, $s0, 1
	move $s0, $t3
	sw $s0, idMain_idResult

	li $v0, 1
	lw $a0, idMain_idResult
	syscall
	lw $s0, idMain_idA
	lw $s1, idMain_idB
	sgt $t0, $s0, $s1
	bne $t0, 1, IF0
	lw $s0, idMain_idA
	lw $s1, idMain_idB
	sub $t1, $s0, $s1
	move $s0, $t1
	sw $s0, idMain_idResult

	li $v0, 1
	lw $a0, idMain_idResult
	syscall
	b IF1
IF0:
	lw $s0, idMain_idB
	lw $s1, idMain_idA
	sub $t0, $s0, $s1
	move $s0, $t0
	sw $s0, idMain_idResult

	li $v0, 1
	lw $a0, idMain_idResult
	syscall
IF1:
	li $t0, 10
	sw $t0, idMain_idLoop

while0:
	lw $s0, idMain_idLoop
	sgt $t0, $s0, 0
	bne $t0, 1, while1
	lw $s0, idMain_idLoop
	sub $t1, $s0, 1
	move $s0, $t1
	sw $s0, idMain_idLoop

	lw $s0, idMain_idLoop
	sgt $t0, $s0, 0
	beq $t0, 1, while0
while1:
	li $v0, 1
	lw $a0, idMain_idLoop
	syscall
	la $s0, idMain_idArray
	li $t0, 5
	sw $t0, 4($s0)

	la $s0, idMain_idArray
	li $t0, 10
	sw $t0, 24($s0)

	la $s0, idMain_idArray
	la $s1, idMain_idArray
	lw $s2, 4($s0)
	lw $s3, 24($s1)
	add $t0, $s2, $s3
	move $s0, $t0
	sw $s0, idMain_idResult

	li $v0, 1
	lw $a0, idMain_idResult
	syscall
	li $v0, 1
	lw $a0, idMain_idResult
	syscall
	li $v0, 10
	syscall
