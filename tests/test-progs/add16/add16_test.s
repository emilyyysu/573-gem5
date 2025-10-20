.LC0:
        .string "RISC-V Packed Addition using 0xFFFFFFFFFFFFFFFF and 0xFFFFFFFFFFFFFFFF "
.LC1:
        .string "Output is 0x%LX \n"
.LC3:
        .string "Test Passed! "
main:
        addi    sp,sp,-48
        sd      ra,40(sp)
        sd      s0,32(sp)
        addi    s0,sp,48
        li      a5,-1
        sd      a5,-24(s0)
        li      a5,-1
        sd      a5,-32(s0)
        sd      zero,-40(s0)
        lui     a5,%hi(.LC0)
        addi    a0,a5,%lo(.LC0)
        call    puts
        ld      a5,-24(s0)
        ld      a4,-32(s0)
        add16 a5, a5,a4

        sd      a5,-40(s0)
        ld      a1,-40(s0)
        lui     a5,%hi(.LC1)
        addi    a0,a5,%lo(.LC1)
        call    printf
        ld      a4,-40(s0)
        lui     a5,%hi(.LC2)
        ld      a5,%lo(.LC2)(a5)
        bne     a4,a5,.L2
        lui     a5,%hi(.LC3)
        addi    a0,a5,%lo(.LC3)
        call    puts
.L2:
        li      a5,0
        mv      a0,a5
        ld      ra,40(sp)
        ld      s0,32(sp)
        addi    sp,sp,48
        jr      ra
.LC2:
        .dword  -281479271743490
