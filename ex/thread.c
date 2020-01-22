#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

const int th_arg = 1234;

void *thread_fn(void *arg) {
  assert(*((int *)arg) == th_arg);

  pthread_t *th = malloc(sizeof(pthread_t));
  assert(th != NULL);

  *th = pthread_self();

  return (void *)th;
}

#define check_error(msg)               \
  if (err) {                           \
    perror(msg); \
    abort();                           \
  }

int main() {
  int err = 0;
  pthread_attr_t tha; // optionaly
  pthread_t th;
  pthread_t *th_res;

  err = pthread_attr_init(&tha);
  check_error("fail: create attributes");

  err = pthread_create(&th, &tha, &thread_fn, (void *)&th_arg);
  check_error("fail: create thread");

  err = pthread_join(th, (void *)&th_res);
  check_error("fail: join thread");
  assert(pthread_equal(*th_res, th));
  free(th_res);
  th_res = NULL;

  err = pthread_attr_setdetachstate(&tha, PTHREAD_CREATE_DETACHED);
  check_error("fail: set attr detach");
  err = pthread_create(&th, &tha, &thread_fn, (void *)&th_arg);
  check_error("fail: create detach thread");

  pthread_attr_destroy(&tha);

  return EXIT_SUCCESS;
}
