typedef unsigned long long uint64_t;

static void print_str(const char *s)
{
    register uint64_t a0 asm("a0") = 1; // fd = 1 (stdout)
    register const char *a1 asm("a1") = s;
    register uint64_t a2 asm("a2") = 0;
    // compute length
    while (s[a2]) a2++;
    register uint64_t a7 asm("a7") = 64; // write syscall
    asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a7));
}

static void print_hex(uint64_t val)
{
    char buf[19];
    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 0; i < 16; i++) {
        int nibble = (val >> ((15 - i) * 4)) & 0xF;
        buf[2 + i] = (nibble < 10) ? ('0' + nibble) : ('a' + nibble - 10);
    }
    buf[18] = '\n';
    buf[19] = 0;
    print_str(buf);
}

void _start()
{
    volatile uint64_t *src = (volatile uint64_t *)0x40000000;
    volatile uint64_t *dst = (volatile uint64_t *)0x40000010;

    volatile uint64_t *ctrl_reg = (uint64_t *)0x60000008;
    volatile uint64_t *dst_reg = (uint64_t *)0x60000004;
    volatile uint64_t *src_reg = (uint64_t *)0x60000000 + 0x00;

    // src[0] = 0x3ff0000000000000;
    // src[1] = 0x3ff0000000000000;
    // src[2] = 0x3ff0000000000000;

    print_str("src addr = ");
    print_hex((uint64_t)src);
    print_str("dst addr = ");
    print_hex((uint64_t)dst);

    print_str("setting source reg:");
    print_hex((uint64_t)src_reg);
    *src_reg = (uint64_t)src;

    print_str("setting dst reg: ");
    print_hex((uint64_t)dst_reg);
    *dst_reg = (uint64_t)dst;

    print_str("setting start bit and length: ");
    *ctrl_reg = (3 * 8) | (1ULL << 31);

    while ((*ctrl_reg & (1UL << 30)) == 0) {}

    print_str("Done\n");

    asm volatile("li a7, 93\nli a0, 0\necall"); // exit(0)
}


// typedef unsigned long long uint64_t;

// volatile double src[3];
// volatile double dst[3];

// void _start() {
//     volatile uint64_t *ctrl = (uint64_t *)0x2F000008;
//     volatile uint64_t *dst_reg = (uint64_t *)0x2F000004;

//     // Initialize src
//     src[0] = *(double*)&(uint64_t){0x3ff0000000000000ULL};
//     src[1] = *(double*)&(uint64_t){0x4000000000000000ULL};
//     src[2] = *(double*)&(uint64_t){0x4008000000000000ULL};

//     // Program accelerator
//     *dst_reg = (uint64_t)dst;
//     *ctrl = (3*8) | (1ULL << 31);

//     while ((*ctrl & (1UL << 30)) == 0) {}

//     // Exit
//     asm volatile("li a7, 93\nli a0, 0\n ecall");
// }
