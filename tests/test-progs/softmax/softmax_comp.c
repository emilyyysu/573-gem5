#include <stdint.h>
#include <stdio.h>
#include <math.h>
int main(void)
{
  float numbers[4] = {1.1, 2.1, 3.1, 4.1};
  float exp_numbers[4];
  float sum = 0;
  for (int i = 0; i < 4; i++) {
    exp_numbers[i] = exp(numbers[i]);
    sum += exp_numbers[i];
  }
  for (int i=0; i<4; i++) {
    numbers[i] = exp_numbers[i]/sum;
  }
  printf("numbers = {%f %f %f %f}", numbers[0], numbers[1], numbers[2], numbers[3]);
}
