typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

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
        int nibble = (val >> ((7 - i) * 4)) & 0xF;
        buf[2 + i] = (nibble < 10) ? ('0' + nibble) : ('a' + nibble - 10);
    }
    buf[10] = '\n';
    buf[11] = 0;
    print_str(buf);
}

void _start()
{
    // Base memory region
    volatile float *src = (volatile float *)0xB0000000;

    // Let's use 512 KiB (enough for simulation but not too huge)
    // Each float = 4 bytes => 512 KiB / 4 = 131072 elements
    const uint32_t N = 100;
    volatile float *dst = src + N;

    volatile uint32_t *src_reg  = (uint32_t *)0xC0000000;
    volatile uint32_t *dst_reg  = (uint32_t *)0xC0000004;
    volatile uint32_t *ctrl_reg = (uint32_t *)0xC0000008;

    // Initialize inputs (0.0, 1.0, 2.0, 3.0, ...)
    for (uint32_t i = 0; i < N; i++) {
        print_hex(i);
        src[i] = (float)2;
    }

    // Program accelerator
    *src_reg = (uint32_t)src;
    *dst_reg = (uint32_t)dst;

    uint32_t total_bytes = N * sizeof(float);
    print_str("Starting large memcpy accel test:\n");
    print_str("Length (bytes): ");
    print_hex(total_bytes);
    *ctrl_reg = total_bytes | (1u << 31);

    // Wait until done bit (bit 30)
    while ((*ctrl_reg & (1u << 30)) == 0) {}

    print_str("Computation complete.\n");

    // Print a few samples to verify
    print_str("Sample outputs:\n");
    print_str("dst[0]   = "); print_hex(*(uint32_t*)&dst[0]);
    print_str("dst[1]   = "); print_hex(*(uint32_t*)&dst[1]);
    print_str("dst[10]  = "); print_hex(*(uint32_t*)&dst[10]);
    print_str("dst[N-1] = "); print_hex(*(uint32_t*)&dst[N-1]);
    print_str("Done.\n");

    // exit(0)
    asm volatile("li a7, 93\nli a0, 0\necall");
}
