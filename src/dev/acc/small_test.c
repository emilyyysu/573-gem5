typedef unsigned long long uint64_t;

void _start() {
    volatile double *src = (double *)0x80000000;
    volatile double *dst = (double *)0x80000030;  // 3 doubles offset (24 bytes)

    volatile uint64_t *ctrl = (uint64_t *)0x2F000008;
    volatile uint64_t *dst_reg = (uint64_t *)0x2F000004;

    // Bit patterns for 1.0, 2.0, 3.0 as doubles
    src[0] = *(double*)&(uint64_t){0x3ff0000000000000ULL}; // 1.0
    src[1] = *(double*)&(uint64_t){0x4000000000000000ULL}; // 2.0
    src[2] = *(double*)&(uint64_t){0x4008000000000000ULL}; // 3.0

    // Set destination address
    *dst_reg = (uint64_t)dst;

    // Start accelerator: length=3 doubles, set start bit
    *ctrl = (3*8) | (1ULL << 31);

    // Wait until done
    while ((*ctrl >> 30) != 1) {}

    while (1) {} // infinite loop
}
