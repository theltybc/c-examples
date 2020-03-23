#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

long int time_ms(void) {
  struct timeval tp;
  gettimeofday(&tp, NULL);
	return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

int main () {
  printf("t = %li\n", time_ms());
  return EXIT_SUCCESS;
}