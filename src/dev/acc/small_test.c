// typedef unsigned int uint32_t;  // 32-bit on RV32

// static void print_str(const char *s)
// {
//     register uint32_t a0 asm("a0") = 1; // fd = 1 (stdout)
//     register const char *a1 asm("a1") = s;
//     register uint32_t a2 asm("a2") = 0;
//     while (s[a2]) a2++;
//     register uint32_t a7 asm("a7") = 64; // write syscall
//     asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a7));
// }

// static void print_hex(uint32_t val)
// {
//     char buf[20];
//     buf[0] = '0';
//     buf[1] = 'x';
//     for (int i = 0; i < 8; i++) {
//         int nibble = (val >> ((7 - i) * 4)) & 0xF;
//         buf[2 + i] = (nibble < 10) ? ('0' + nibble) : ('a' + nibble - 10);
//     }
//     buf[10] = '\n';
//     buf[11] = 0;
//     print_str(buf);
// }

// void _start()
// {
//     volatile uint32_t *src = (volatile uint32_t *)0x40000000;
//     volatile uint32_t *dst = (volatile uint32_t *)0x40000010;

//     volatile uint32_t *src_reg  = (volatile uint32_t *)0x60000000;
//     volatile uint32_t *dst_reg  = (volatile uint32_t *)0x60000004;
//     volatile uint32_t *ctrl_reg = (volatile uint32_t *)0x60000008;

//     src[0] = 0x1;
//     src[1] = 0x2;

//     print_str("src addr = ");
//     print_hex((uint32_t)src);
//     print_str("dst addr = ");
//     print_hex((uint32_t)dst);

//     print_str("setting source reg: ");
//     print_hex((uint32_t)src_reg);
//     *src_reg = (uint32_t)src;

//     print_str("setting dst reg: ");
//     print_hex((uint32_t)dst_reg);
//     *dst_reg = (uint32_t)dst;

//     print_str("setting start bit and length: ");
//     print_hex((uint32_t)ctrl_reg);
//     *ctrl_reg = (8) | (1U << 31);   // start=bit31, len=8 bytes

//     while ((*ctrl_reg & (1U << 30)) == 0) {
//         // wait for done bit
//     }

//     print_str("Checking src: ");
//     print_hex(src[0]);
//     print_hex(src[1]);

//     print_str("Checking dst: ");
//     print_hex(dst[0]);
//     print_hex(dst[1]);

//     print_str("Expected: 1 2\n");
//     print_str("Done\n");

//     asm volatile("li a7, 93\nli a0, 0\necall"); // exit(0)
// }


typedef unsigned long uint32_t;

static void print_str(const char *s)
{
    register uint32_t a0 asm("a0") = 1; // fd = 1 (stdout)
    register const char *a1 asm("a1") = s;
    register uint32_t a2 asm("a2") = 0;
    // compute length
    while (s[a2]) a2++;
    register uint32_t a7 asm("a7") = 64; // write syscall
    asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a7));
}

static void print_hex(uint32_t val)
{
    char buf[19];
    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 0; i < 8; i++) {
        int nibble = (val >> ((15 - i) * 4)) & 0xF;
        buf[2 + i] = (nibble < 10) ? ('0' + nibble) : ('a' + nibble - 10);
    }
    buf[18] = '\n';
    buf[19] = 0;
    print_str(buf);
}

void _start()
{
    volatile float *src = (volatile float *)0x40000000;
    volatile float *dst = (volatile float *)0x40000010;

    volatile uint32_t *ctrl_reg = (uint32_t *)0x60000008;
    volatile uint32_t *dst_reg = (uint32_t *)0x60000004;
    volatile uint32_t *src_reg = (uint32_t *)0x60000000 + 0x00;

    src[0] = 0.0f;
    src[1] = 1.0f;

    print_str("src addr = ");
    print_hex((uint32_t)src);
    print_str("dst addr = ");
    print_hex((uint32_t)dst);

    print_str("setting source reg:");
    print_hex((uint32_t)src_reg);
    *src_reg = (uint32_t)src;

    print_str("setting dst reg: ");
    print_hex((uint32_t)dst_reg);
    *dst_reg = (uint32_t)dst;

    print_str("setting start bit and length: ");
    *ctrl_reg = (8) | (1ULL << 31);
    //while (1) {}
    while ((*ctrl_reg & (1UL << 30)) == 0) {}

    print_str("Checking src: ");
    print_hex((uint32_t)src[0]);
    print_str(" ");
    print_hex((uint32_t)src[1]);
    // print_str(" ");
    // print_hex((uint32_t)src[2]);
    print_str("\n");
    

    print_str("Checking dst: ");
    print_hex(*(unsigned long long *)&dst[0]);
    print_str(" ");
    print_hex(*(unsigned long long *)&dst[1]);
    // print_str(" ");
    // print_hex((uint32_t)dst[2]);
    print_str("\n");
    
    print_str("Expected: e^0 e^1\n");
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
