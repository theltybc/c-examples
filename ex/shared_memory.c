#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define check_error(val, msg) \
  do {                        \
    if ((val) == -1) {        \
      perror((msg));          \
      exit(EXIT_FAILURE);     \
    }                         \
  } while (0);

void chaild(key_t msgkey);
void parent(key_t msgkey);

int open_segment(key_t keyval, int segsize);
int close_segment(int shmqid);
void *attach_segment(int shmid);
int detach_segment(void *shmaddr);

#define BUFFER_SIZE 1000
const char *example_string = "1234567890";

int main() {
  key_t msgkey;

  msgkey = ftok(".", 'm');
  check_error(msgkey, "Generate our IPC key");

  switch (fork()) {
  case -1:
    perror("fork");
    abort();
  case 0:
    chaild(msgkey);
    exit(EXIT_SUCCESS);
  default:
    parent(msgkey);
  }

  return EXIT_SUCCESS;
}

void chaild(key_t msgkey) {
  int shmid = open_segment(msgkey, BUFFER_SIZE);
  check_error(shmid, "chaild: open mem");

  char *shmaddr = (char *)attach_segment(shmid);
  if (shmaddr == NULL) {
    perror("chaild: attach segment");
    exit(EXIT_FAILURE);
  }
  strncpy(shmaddr, example_string, BUFFER_SIZE);
  shmaddr[BUFFER_SIZE - 1] = '\0';
  check_error(detach_segment(shmaddr), "chaild: detach_segment");
}

void parent(key_t msgkey) {
  int shmid = open_segment(msgkey, BUFFER_SIZE);
  check_error(shmid, "parent: open mem");

  char *shmaddr = (char *)attach_segment(shmid);
  if (shmaddr == NULL) {
    perror("parent: attach segment");
    exit(EXIT_FAILURE);
  }
  wait(NULL);

  if (strncmp(shmaddr, example_string, BUFFER_SIZE) == 0) {
    printf("parent: example_string eq\n");
  } else {
    fprintf(stderr, "parennt: example_string NOT eq\n");
    check_error(detach_segment(shmaddr), "parent: detach_segment");
    check_error(close_segment(shmid), "parent: close_segment");
    exit(EXIT_FAILURE);
  }
  check_error(detach_segment(shmaddr), "parent: detach_segment");
  check_error(close_segment(shmid), "parent: close_segment");
}

int open_segment(key_t keyval, int segsize) {
  return shmget(keyval, segsize, IPC_CREAT | 0660);
}

int close_segment(int shmqid) {
  return shmctl(shmqid, IPC_RMID, NULL);
}

void *attach_segment(int shmid) {
  return shmat(shmid, 0, 0);
}

int detach_segment(void *shmaddr) {
  return shmdt(shmaddr);
}