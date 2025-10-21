typedef unsigned long long uint64_t;

void _start() {
    volatile double src[3];
    volatile double dst[3];

    volatile uint64_t *ctrl = (uint64_t *)0x2F000008;
    volatile uint64_t *dst_reg = (uint64_t *)0x2F000004;

    // Assign doubles using integer bit pattern to avoid .rodata
    src[0] = *(double*)&(uint64_t){0x3ff0000000000000ULL}; // 1.0
    src[1] = *(double*)&(uint64_t){0x4000000000000000ULL}; // 2.0
    src[2] = *(double*)&(uint64_t){0x4008000000000000ULL}; // 3.0

    *dst_reg = (uint64_t)dst;    
    *ctrl = (3*8) | (1ULL << 31);

        // wait for done bit (poll)
    while ((*ctrl & (1UL << 30)) == 0) {}

    // exit cleanly
    asm volatile("li a7, 93\nli a0, 0\n ecall");  // RISC-V exit syscall
    return 0;
}
