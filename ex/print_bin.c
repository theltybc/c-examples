#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void __attribute__((unused)) print_bin(void *mem, size_t size) {
  char buff[9];
  size_t byte = 0;
  uint8_t bit;
  while (byte < size) {
    bit = 8;
    while (bit > 0) {
      bit--;
      if (((char *)mem)[byte] & (1 << bit)) {
        buff[7 - bit] = '1';
      } else {
        buff[7 - bit] = '0';
      }
    }
    buff[8] = '\0';
    printf(" %s", buff);
    byte++;
  }
}

int main() {
  uint8_t str[] = {1, 2, 3, 4, 5, 6};
  print_bin(str, sizeof(str));
  printf("\n");
  return EXIT_SUCCESS;
}