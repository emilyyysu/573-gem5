#include <stdint.h>
#include <stdio.h>

int main() {
    int value = 42;
    int loaded;

    asm volatile (
        "lw %0, 0(%1)"      // load word: %0 = *(%1)
        : "=r"(loaded)      // output operand (%0)
        : "r"(&value)       // input operand (%1)
    );

    printf("Loaded value: %d\n", loaded);
    return 0;
}
