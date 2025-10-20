	.file	"softmax_comp.c"
	.option nopic
	.attribute arch, "rv32i2p1_m2p0_a2p1_f2p2_d2p2_c2p0_zicsr2p0_zifencei2p0_zmmul1p0_zaamo1p0_zalrsc1p0_zca1p0_zcd1p0_zcf1p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.section	.rodata
	.align	2
.LC1:
	.string	"numbers = {%f %f %f %f}"
	.align	2
.LC0:
	.word	1066192077
	.word	1074161254
	.word	1078355558
	.word	1082340147
	.text
	.align	1
	.globl	main
	.type	main, @function
main:
	addi	sp,sp,-96
	sw	ra,92(sp)
	sw	s0,88(sp)
	addi	s0,sp,96
	lui	a5,%hi(.LC0)
	addi	a5,a5,%lo(.LC0)
	lw	a2,0(a5)
	lw	a3,4(a5)
	lw	a4,8(a5)
	sw	a2,-44(s0)
	sw	a3,-40(s0)
	sw	a4,-36(s0)
	lw	a5,12(a5)
	sw	a5,-32(s0)
	sw	zero,-20(s0)
	sw	zero,-24(s0)
	j	.L2
.L3:
	lw	a4,-24(s0)
	addi	a5,s0,-44
	slli	a4,a4,2
	add	a5,a4,a5
	flw	fa5,0(a5)
	fcvt.d.s	fa5,fa5
	fsd	fa5,-72(s0)
	lw	a0,-72(s0)
	lw	a1,-68(s0)
	call	exp
	sw	a0,-72(s0)
	sw	a1,-68(s0)
	fld	fa5,-72(s0)
	fcvt.s.d	fa5,fa5
	lw	a4,-24(s0)
	addi	a5,s0,-60
	slli	a4,a4,2
	add	a5,a4,a5
	fsw	fa5,0(a5)
	lw	a4,-24(s0)
	addi	a5,s0,-60
	slli	a4,a4,2
	add	a5,a4,a5
	flw	fa5,0(a5)
	flw	fa4,-20(s0)
	fadd.s	fa5,fa4,fa5
	fsw	fa5,-20(s0)
	lw	a5,-24(s0)
	addi	a5,a5,1
	sw	a5,-24(s0)
.L2:
	lw	a4,-24(s0)
	li	a5,3
	ble	a4,a5,.L3
	sw	zero,-28(s0)
	j	.L4
.L5:
	lw	a4,-28(s0)
	addi	a5,s0,-60
	slli	a4,a4,2
	add	a5,a4,a5
	flw	fa4,0(a5)
	flw	fa5,-20(s0)
	fdiv.s	fa5,fa4,fa5
	lw	a4,-28(s0)
	addi	a5,s0,-44
	slli	a4,a4,2
	add	a5,a4,a5
	fsw	fa5,0(a5)
	lw	a5,-28(s0)
	addi	a5,a5,1
	sw	a5,-28(s0)
.L4:
	lw	a4,-28(s0)
	li	a5,3
	ble	a4,a5,.L5
	flw	fa5,-44(s0)
	fcvt.d.s	fa4,fa5
	flw	fa5,-40(s0)
	fcvt.d.s	fa3,fa5
	flw	fa5,-36(s0)
	fcvt.d.s	fa2,fa5
	flw	fa5,-32(s0)
	fcvt.d.s	fa5,fa5
	fsd	fa5,0(sp)
	fsd	fa2,-72(s0)
	lw	a6,-72(s0)
	lw	a7,-68(s0)
	fsd	fa3,-72(s0)
	lw	a4,-72(s0)
	lw	a5,-68(s0)
	fsd	fa4,-72(s0)
	lw	a2,-72(s0)
	lw	a3,-68(s0)
	lui	a1,%hi(.LC1)
	addi	a0,a1,%lo(.LC1)
	call	printf
	li	a5,0
	mv	a0,a5
	lw	ra,92(sp)
	lw	s0,88(sp)
	addi	sp,sp,96
	jr	ra
	.size	main, .-main
	.ident	"GCC: () 15.1.0"
	.section	.note.GNU-stack,"",@progbits
