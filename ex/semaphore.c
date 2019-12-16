#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
// #include <linux/msg.h>
#include <assert.h>

#define check_error(val, msg) \
  do {                        \
    if ((val) == -1) {        \
      perror((msg));          \
      abort();                \
    }                         \
  } while (0);

key_t get_key(void);
int open_semaphore(key_t keyval, int numsems);
int semaphore_unlock(int sid);
int semaphore_lock(int sid);
int remove_semaphore(int sid, int numsems);

int main() {
  key_t key = get_key();
  int numsems = 5;

  int sid = open_semaphore(key, numsems);
  check_error(sid, "open semaphore");

  check_error(semaphore_unlock(sid), "sem_unlock");

  check_error(semaphore_lock(sid), "sem_lock");

  remove_semaphore(sid, numsems);

  return EXIT_SUCCESS;
}

key_t get_key(void) {
  key_t key;

  key = ftok(".", 'm');
  check_error(key, "Generate our IPC key");
  return key;
}

int open_semaphore(key_t keyval, int numsems) {
  if (!numsems) {
    return (-1);
  }
  return semget(keyval, numsems, IPC_CREAT | 0660);
}

int semaphore_unlock(int sid) {
  struct sembuf sem_unlock = {0, 1, IPC_NOWAIT};
  return semop(sid, &sem_unlock, 1);
}

int semaphore_lock(int sid) {
  struct sembuf sem_lock = {0, -1, IPC_NOWAIT};
  return semop(sid, &sem_lock, 1);
}

int remove_semaphore(int sid, int numsems) {
  return semctl(sid, numsems, IPC_RMID);
}

