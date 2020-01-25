#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdint.h>

#define check_error(msg) \
  if (err) {             \
    perror(msg);         \
    abort();             \
  }

int err = 0;
const int th_arg = 1234;
pthread_key_t key;
int on_exit_thread_called = 0;
int detached_thread_fn_sleep = 0;

void *thread_fn(void *arg) {
  assert(*((int *)arg) == th_arg);

  pthread_t *th = malloc(sizeof(pthread_t));
  assert(th != NULL);

  *th = pthread_self();
  pthread_exit((void *)th);
  return (void *)th;
}

void *detached_thread_fn(void *arg) {
  assert(*((int *)arg) == th_arg);

  err = pthread_setspecific(key, arg);
  check_error("fail: thread specific key")

  detached_thread_fn_sleep = 1;
  sleep(INT32_MAX);
  return NULL;
}

void on_exit_thread(void *arg) {
  assert(*((int *)arg) == th_arg);
  on_exit_thread_called = 1;
}

int main() {
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

  err = pthread_key_create(&key, &on_exit_thread);
  check_error("fail: set key pthread");
  err = pthread_attr_setdetachstate(&tha, PTHREAD_CREATE_DETACHED);
  check_error("fail: set attr detached");
  err = pthread_create(&th, &tha, &detached_thread_fn, (void *)&th_arg);
  check_error("fail: create detached thread");

  pthread_attr_destroy(&tha);

  while (!detached_thread_fn_sleep) {
    usleep(1);
  }

  err = pthread_cancel(th);
  check_error("fail: cancel detached thread");

  while (!on_exit_thread_called) {
    usleep(1);
  }

  return EXIT_SUCCESS;
}
