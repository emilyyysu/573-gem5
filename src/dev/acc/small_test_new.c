typedef unsigned long uint32_t;

static void print_str(const char *s)
{
    register uint32_t a0 asm("a0") = 1;
    register const char *a1 asm("a1") = s;
    register uint32_t a2 asm("a2") = 0;
    while (s[a2]) a2++;
    register uint32_t a7 asm("a7") = 64;
    asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a7));
}

static void print_hex(uint32_t val)
{
    char buf[19];
    buf[0] = '0'; buf[1] = 'x';
    for (int i = 0; i < 8; i++) {
        int nibble = (val >> ((15 - i) * 4)) & 0xF;
        buf[2 + i] = (nibble < 10) ? ('0' + nibble) : ('a' + nibble - 10);
    }
    buf[10] = '\n';
    buf[11] = 0;
    print_str(buf);
}

void _start()
{
    volatile float *src = (volatile float *)0xB0000000;
    volatile float *dst = (volatile float *)0xB0000010;

    volatile uint32_t *ctrl_reg = (uint32_t *)0xC0000008;
    volatile uint32_t *dst_reg  = (uint32_t *)0xC0000004;
    volatile uint32_t *src_reg  = (uint32_t *)0xC0000000;

    // ---- Input data ----
    src[0] =  0.0f;   // kept
    src[1] = -30.0f;  // pruned (below cutoff)
    src[2] =  1.0f;   // kept
    src[3] = -50.0f;  // pruned (below cutoff)

    print_str("Loaded src values:\n");
    print_hex(*(uint32_t *)&src[0]);
    print_hex(*(uint32_t *)&src[1]);
    print_hex(*(uint32_t *)&src[2]);
    print_hex(*(uint32_t *)&src[3]);

    // ---- Program accelerator ----
    *src_reg = (uint32_t)src;
    *dst_reg = (uint32_t)dst;

    print_str("Starting accelerator...\n");
    *ctrl_reg = (16) | (1UL << 31);

    while ((*ctrl_reg & (1UL << 30)) == 0) { }

    print_str("DMA done.\n");

    // ---- Read results ----
    print_str("DST values:\n");
    print_hex(*(uint32_t *)&dst[0]);
    print_hex(*(uint32_t *)&dst[1]);
    print_hex(*(uint32_t *)&dst[2]);
    print_hex(*(uint32_t *)&dst[3]);

    // ---- Expected behavior ----
    print_str("Expected:\n");
    print_str(" dst[0] = softmax(0)\n");
    print_str(" dst[1] = epsilon (pruned)\n");
    print_str(" dst[2] = softmax(1)\n");
    print_str(" dst[3] = epsilon (pruned)\n");

    asm volatile("li a7, 93\nli a0, 0\necall");
}
