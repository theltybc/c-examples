#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

long int time_ms(void) {
  static_assert(sizeof(long int) >= 8, "long to small for store time");
  struct timeval tp;
  gettimeofday(&tp, NULL);
  return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

int main() {
  printf("t = %li\n", time_ms());
  return EXIT_SUCCESS;
}
