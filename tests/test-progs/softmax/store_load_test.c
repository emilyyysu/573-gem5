#include <stdio.h>

int main(void) {
    int x = 123;
    int y = 0;

    // store x into &y
    asm volatile(
        "sw %1, 0(%0)"      // *%0 = %1   (store word)
        :                    // no outputs
        : "r"(&y), "r"(x)    // inputs: %0 = &y, %1 = x
        : "memory"           // assembly touches memory
    );

    // load y back into z
    int z;
    asm volatile(
        "lw %0, 0(%1)"       // %0 = *%1   (load word)
        : "=r"(z)             // output: z
        : "r"(&y)             // input: &y
    );

    printf("z = %d\n", z);
    int numbers[4] = {1, 2, 3, 4};
    int out[4] = {0, 0, 0, 0};
    for (int i = 0; i <= 3; i++) {
        asm volatile("lw %0, 0(%1)" : "=r"(out[i]) : "r"(&numbers[i]));
        printf("out = %d, mem_addr = %d\n", out[i], &numbers[i]);
    }

    return 0;
}
