    .section .data
values:
    .word 1, 2, 3, 4          # 4 contiguous integers in memory

    .section .text
    .globl _start
_start:
    # Load base address of the array into x1
    la   x1, values           # x1 = &values[0]

    # Load number of elements into x2
    li   x2, 4

    ##################################################
    # Custom softmax instruction placeholder
    # Replace 0xXXXXXXXX with your actual encoding
    ##################################################

    .word 0x02208183           # softmax x10, x11 (binary encoding)

    ##################################################
    # Optional: read results back
    ##################################################
    lw   x12, 0(x1)
    lw   x13, 4(x1)
    lw   x14, 8(x1)
    lw   x15, 12(x1)

    # Exit (for gem5 / pk)
    li   a7, 93
    ecall
