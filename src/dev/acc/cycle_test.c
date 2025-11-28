typedef unsigned int  uint32_t;
typedef unsigned long long uint64_t;

static inline uint64_t rdcycle()
{
    uint64_t x;
    asm volatile("rdcycle %0" : "=r"(x));
    return x;
}

static void print_str(const char *s)
{
    register uint32_t a0 asm("a0") = 1;
    register const char *a1 asm("a1") = s;
    register uint32_t a2 asm("a2") = 0;
    while (s[a2]) a2++;
    register uint32_t a7 asm("a7") = 64;
    asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a7));
}

static void print_hex(uint32_t v)
{
    char b[12];
    b[0]='0'; b[1]='x';
    for(int i=0;i<8;i++){
        int n=(v>>((7-i)*4))&0xF;
        b[2+i]=n<10?'0'+n:'a'+(n-10);
    }
    b[10]='\n'; b[11]=0;
    print_str(b);
}

static void print_dec(uint64_t v)
{
    char b[32];
    int i=30; b[31]=0;
    if(v==0){ print_str("0\n"); return; }
    for(;v>0;i--){
        b[i]='0' + (v%10);
        v/=10;
    }
    print_str(&b[i+1]);
    print_str("\n");
}

void run_case(const char *name, float *src, float *dst, uint32_t N)
{
    volatile uint32_t *SRC = (uint32_t*)0xC0000000;
    volatile uint32_t *DST = (uint32_t*)0xC0000004;
    volatile uint32_t *CTL = (uint32_t*)0xC0000008;

    print_str("\n--- ");
    print_str(name);
    print_str(" ---\n");

    // Program accelerator
    *SRC = (uint32_t)src;
    *DST = (uint32_t)dst;

    uint32_t bytes = N * 4;

    // Measure start cycle
    uint64_t c0 = rdcycle();

    // Launch
    *CTL = bytes | (1u<<31);

    // Wait
    while((*CTL & (1u<<30)) == 0){}

    uint64_t c1 = rdcycle();

    uint64_t cycles = c1 - c0;

    print_str("Measured cycles: ");
    print_dec(cycles);

    // Print sample outputs
    print_str("dst[0]  = "); print_hex(*(uint32_t*)&dst[0]);
    print_str("dst[N-1]= "); print_hex(*(uint32_t*)&dst[N-1]);
}

void _start()
{
    const uint32_t N = 128;
    volatile float *src = (volatile float*)0xB0000000;
    volatile float *dst = src + N;

    float max = 1000.0f;

    //-----------------------------
    // Case A: No pruning
    //-----------------------------
    for(uint32_t i=0;i<N;i++)
        src[i] = max - 0.1f;
    run_case("No pruning", (float*)src, (float*)dst, N);

    //-----------------------------
    // Case B: Full pruning
    //-----------------------------
    for(uint32_t i=0;i<N;i++)
        src[i] = max - 1000.0f;
    src[0] = max;
    run_case("Full pruning", (float*)src, (float*)dst, N);

    //-----------------------------
    // Case C: Partial pruning
    //-----------------------------
    for(uint32_t i=0;i<64;i++)
        src[i] = max - 10.0f;   // kept
    for(uint32_t i=64;i<N;i++)
        src[i] = max - 50.0f;   // pruned
    src[0] = max;               // max_x
    run_case("Partial pruning", (float*)src, (float*)dst, N);

    // exit
    asm volatile("li a7,93; li a0,0; ecall");
}
