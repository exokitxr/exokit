#include <stdio.h>
#include <stdint.h>

void consoleLogI(u32 n) {
  printf("%d\n", n);
}

void (*Z_consoleZ_logiZ_vi)(u32) = consoleLogI;
