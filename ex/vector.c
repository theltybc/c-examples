#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct Vector {
  size_t len;
  size_t size;
  size_t item_size;
  void *arr;
} Vector;

Vector *vec_new(size_t init_size, size_t item_size) {
  Vector *vec = (Vector *)malloc(sizeof(Vector));
  if (vec == NULL) {
    return NULL;
  }
  vec->len = 0;
  vec->size = init_size > 0 ? init_size : 1;
  vec->item_size = item_size;
  vec->arr = malloc(vec->size * vec->item_size);
  if (vec->arr == NULL) {
    free(vec);
    return NULL;
  }
  return vec;
}

void *vec_push(Vector *vec, void *val) {
  if (vec->size <= vec->len) {
    size_t size = vec->size * 2 * vec->item_size;
    void *tmp = realloc(vec->arr, size);
    if (tmp == NULL) {
      return NULL;
    }
    vec->size *= 2;
    vec->arr = tmp;
  }
  void *pos = (char *)vec->arr + vec->len * vec->item_size;
  memcpy(pos, val, vec->item_size);
  vec->len++;
  return pos;
}

void *vec_get(Vector *vec, size_t index) {
  if (index >= vec->len) {
    return NULL;
  }
  return (char *)vec->arr + vec->item_size * index;
}

void *vec_pop(Vector *vec) {
  if (vec->len == 0) {
    return NULL;
  }
  vec->len--;
  return (char *)vec->arr + vec->item_size * vec->len;
}

void vec_free(Vector *vec) {
  vec->len = 0;
  vec->size = 0;
  vec->item_size = 0;
  free(vec->arr);
  vec->arr = NULL;
  free(vec);
}

int main() {
  Vector *vec = vec_new(3, sizeof(double));
  if (vec == NULL) {
    perror("fail: init vector");
    return EXIT_FAILURE;
  }
  int i;
  for (i = 0; i < 1000000; i++) {
    int *i_ptr = vec_push(vec, &i);
    assert(i_ptr != NULL && i == *i_ptr && vec_get(vec, i) != NULL && *(int *)vec_get(vec, i) == i);
  }
  for (i--; i >= 0; i--) {
    int *i_ptr = vec_pop(vec);
    assert(i_ptr != NULL && *i_ptr == i);
  }
  vec_free(vec);
  return EXIT_SUCCESS;
}