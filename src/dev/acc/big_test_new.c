typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

/* ------------------ PRINT HELPERS ------------------ */

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
    char buf[12];
    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 0; i < 8; i++) {
        uint32_t nibble = (val >> ((7 - i) * 4)) & 0xF;
        buf[2 + i] = (nibble < 10) ? ('0' + nibble) : ('a' + nibble - 10);
    }
    buf[10] = '\n';
    buf[11] = 0;
    print_str(buf);
}

/* ------------------ MAIN TEST ------------------ */

void _start()
{
    volatile float *src = (volatile float *)0xB0000000;

    const uint32_t N = 256;   // Bigger test
    volatile float *dst = src + N;   // output starts right after input

    volatile uint32_t *src_reg  = (uint32_t *)0xC0000000;
    volatile uint32_t *dst_reg  = (uint32_t *)0xC0000004;
    volatile uint32_t *ctrl_reg = (uint32_t *)0xC0000008;

    /* ------------------------------------------------
       Initialize input tensor with a rich distribution:
       - Region 0–63: increasing values 0..63
       - Region 64–127: negative decreasing values
       - Region 128–191: constant very negative (prune)
       - Region 192–255: random-ish pattern
       ------------------------------------------------ */
    print_str("Initializing input array...\n");

    for (uint32_t i = 0; i < 64; i++)
        src[i] = (float)i;

    for (uint32_t i = 64; i < 128; i++)
        src[i] = -(float)(i - 64) * 0.5f;

    for (uint32_t i = 128; i < 192; i++)
        src[i] = -50.0f;       // should be strongly pruned (shifted < -20)

    for (uint32_t i = 192; i < 256; i++)
        src[i] = (float)((i * 37) % 23) - 5.0f;   // pseudo-random small values

    /* ------------------------------------------------
       Program accelerator
       ------------------------------------------------ */
    *src_reg = (uint32_t)src;
    *dst_reg = (uint32_t)dst;

    uint32_t total_bytes = N * sizeof(float);

    print_str("Launching memcpy accelerator...\n");
    print_str("Total bytes = ");
    print_hex(total_bytes);

    *ctrl_reg = total_bytes | (1u << 31);

    /* ------------------------------------------------
       Wait for done bit (bit 30).
       ------------------------------------------------ */
    while ((*ctrl_reg & (1u << 30)) == 0) {}

    print_str("Compute phase finished.\n");

    /* ------------------------------------------------
       Print representative sample outputs
       ------------------------------------------------ */
    print_str("Sample outputs:\n");

    print_str("dst[0] = ");
    print_hex(*(uint32_t *)&dst[0]);

    print_str("dst[10] = ");
    print_hex(*(uint32_t *)&dst[10]);

    print_str("dst[63] = ");
    print_hex(*(uint32_t *)&dst[63]);

    print_str("dst[100] (neg block) = ");
    print_hex(*(uint32_t *)&dst[100]);

    print_str("dst[150] (pruned block) = ");
    print_hex(*(uint32_t *)&dst[150]);

    print_str("dst[200] (random block) = ");
    print_hex(*(uint32_t *)&dst[200]);

    print_str("dst[N-1] = ");
    print_hex(*(uint32_t *)&dst[N-1]);

    print_str("Test complete.\n");

    asm volatile("li a7, 93\nli a0, 0\necall"); // exit(0)
}
