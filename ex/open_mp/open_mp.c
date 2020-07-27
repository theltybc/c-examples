#include "omp.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
  int value = 0b0;

  printf("start\n");
#pragma omp parallel num_threads(2)
  {
#pragma omp atomic
    value++;
#pragma omp critical(cout)
    {
#ifdef _OPENMP
      printf("th: %d; value: %d\n", omp_get_thread_num(), value);
#else
      printf("th: main; value: %d\n", value);
#endif
    }
  }
  printf("finish\n\n");

  printf("start\n");
  int i = 0;
#pragma omp parallel for
  for (i = 0; i < 10; i++) {
    printf("iteration %d, thread=%d\n", i, omp_get_thread_num());
  }
  printf("finish\n\n");

  printf("start\n");
#pragma omp parallel sections private(i)
  {
#pragma omp section
    {
      for (i = 0; i < 10; i++)
        printf("iteration %d\n", i);
    }
#pragma omp section
    {
      for (i = 10; i < 20; i++)
        printf("iteration %d\n", i);
    }
  }
  printf("finish\n\n");

  return EXIT_SUCCESS;
}
