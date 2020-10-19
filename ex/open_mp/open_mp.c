#include "omp.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>

static int is_prime(uint64_t num) {
  uint64_t mid = (uint64_t)sqrt((double)num);
  // printf("num: %lu mid: %lu \n", num, mid);
  for (uint64_t i = 2; i <= mid; i++) {
    if ((num % i) == 0) {
      return 0;
    }
  }
  return 1;
}

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

  uint64_t count = 1000000;
  uint64_t num = 1;
  uint64_t primes[count];
  uint64_t cur = 0;

#pragma omp parallel for
  for (num = 1; num < 10000000; num++) {
    if (is_prime(num)) {
#pragma omp critical(push)
      {
        primes[cur] = num;
        cur++;
      }
    }
  }

  srand(time(NULL));
#pragma omp parallel for
  for (num = 0; num < cur; num++) {
    assert(is_prime(primes[cur] * primes[rand() % cur]));
  }

  return EXIT_SUCCESS;
}
