#include <stdint.h>
#include <stdio.h>
int main(void)
{
  float numbers[4] = {1, 2, 3, 4};
  int out[4] = {0, 0, 0, 0};

  for (int i = 0; i < 4; i++) {
      asm volatile("lw %0, 0(%1)" : "=r"(out[i]) : "r"(&numbers[i]));
      printf("out = %d, mem_addr = %x\n", out[i], &numbers[i]);
  }
  int base = (int)(&numbers[0]);
  printf("base = %x\n", base);
  int num_elements = 4;

  int sum = 0;
  printf("RISC-V Softmax using 0x1, 0x2, 0x3, 0x4 \n");
  asm volatile("divu %0, %1,%2\n":"=r"(sum):"r"(base),"r"(num_elements));
  printf("Sum is %x\n", sum);
  for (int i = 0; i < num_elements; i++) {
    printf("%i %f", i, numbers[i]);
  }

  return 0;
}
