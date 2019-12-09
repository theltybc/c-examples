#pragma once

#include <stdio.h>

#define TEST
#ifdef TEST
void test(int assertion, char msg[]) {
  if (assertion) {
    printf("\x1b[32m OK \x1b[0m %s\n", msg);
  } else {
    fprintf(stderr, "\x1b[31mFAIL\x1b[0m %s\n", msg);
  }
}

#define test(a) test((a), (#a))
#define testm(a, m) test((a), (m))
#else
#define test(a)
#define testm(a, m)
#endif