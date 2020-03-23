#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>

int my_random() {
  srand(time(NULL) + getpid());
  return rand();
}

int main() {
  printf("t = %i\n", my_random());
  return EXIT_SUCCESS;
}