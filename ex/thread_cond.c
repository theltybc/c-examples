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
int global_val = 1;

typedef struct {
  pthread_cond_t fc;
  pthread_mutex_t fm;
} Flag;

void init_flags(Flag *fl) {
  pthread_cond_init(&(fl->fc), NULL);
  pthread_mutex_init(&(fl->fm), NULL);
}

void *thread_fn(void *flag) {
  Flag *fl = flag;
  while (1) {
    pthread_mutex_lock(&(fl->fm));
    pthread_cond_wait(&(fl->fc), &(fl->fm));
    if (global_val > 10000) {
      global_val = 0;
      pthread_mutex_unlock(&(fl->fm));
      pthread_exit(NULL);
    }
    pthread_mutex_unlock(&(fl->fm));
  }

  return NULL;
}

int main() {
  pthread_attr_t tha; // optionaly
  pthread_t th;
  Flag fl;
  init_flags(&fl);

  err = pthread_attr_init(&tha);
  check_error("fail: create attributes");

  err = pthread_create(&th, &tha, &thread_fn, &fl);
  check_error("fail: create thread");

  while (1) {
    pthread_mutex_lock(&(fl.fm));
    if (global_val == 0) {
      break;
    }
    global_val++;
    pthread_cond_signal(&(fl.fc));
    pthread_mutex_unlock(&(fl.fm));
  }

  err = pthread_join(th, NULL);
  check_error("fail: join thread");

  pthread_attr_destroy(&tha);

  return EXIT_SUCCESS;
}
