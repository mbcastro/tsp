#include <stdio.h>
#include <unistd.h>

int main (void) {
  unsigned long sz = sysconf(_SC_PAGESIZE);
  printf("%lu", sz);
  return 0;
}
