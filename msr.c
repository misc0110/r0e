#include "r0e.h"
#include <stdio.h>

volatile size_t msr = 0;

size_t read_msr() {
  size_t low = 0, high = 0;
  asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
  return low | (high << 32);
}

int main() {
  msr = 0x1a0;

  if (r0e_init()) {
    printf("Could not initialize r0e\n");
    return 1;
  }

  printf("[r0e] Reading MSR 0x%zx\n", msr);
  size_t msrval = r0e_call(read_msr);

  printf("[r0e] MSR(0x%zx) = 0x%zx\n", msr, msrval);

  r0e_cleanup();
  printf("[r0e] Done!\n");
  return 0;
}
