#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <iconv.h>
#include <string.h>
#include <wchar.h>

/* iconv --list - show list code */
#define CODE_FROM "UTF8"
#define CODE_TO "UTF16"

#define SIZE_BUFF 1000

const char test_str[] = "test string 123456789";

int main() {
  iconv_t cd = iconv_open(CODE_TO, CODE_FROM);

  char *in = malloc(sizeof(test_str));
  if (in == NULL) {
    perror("malloc in");
    abort();
  }

  char *out = malloc(SIZE_BUFF);
  if (out == NULL) {
    perror("malloc out");
    abort();
  }

  strcpy(in, test_str);

  size_t len_in = sizeof(test_str) - 1; // exclude '\0'
  char *in_p = in;
  size_t len_out = SIZE_BUFF;
  char *out_p = out;
  if (iconv(cd, &in_p, &len_in, &out_p, &len_out) || len_in != 0) {
    perror("iconv");
  }

  assert(strcmp(test_str, in) == 0);

  FILE *fd = fopen(".test_text", "w");
  if (fd == NULL) {
    perror("fail open file");
  }

  if (fwrite(out, sizeof(char), SIZE_BUFF - len_out, fd) < SIZE_BUFF - len_out) {
    perror("fail write file");
  }


  fclose(fd);
  free(out);
  free(in);
  iconv_close(cd);
  return EXIT_SUCCESS;
}
