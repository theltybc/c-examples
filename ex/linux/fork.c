#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>

#define RETURN_CODE 111 // some number

static_assert(RETURN_CODE < (1 << 8) && RETURN_CODE > 0, "RETURN_CODE out of range"); // is UNIX standart?

int main() {
  pid_t pid;
  int rv;
  switch (pid = fork()) {
  case -1:
    perror("fork");
    exit(EXIT_FAILURE);
  case 0:
    printf(" CHILD: PID -- %d\n", getpid());
    printf(" CHILD: PPID -- %d\n", getppid());
    exit(RETURN_CODE);
  default:
    printf("PARENT: PID -- %d\n", getpid());
    printf("PARENT: PID моего потомка %d\n", pid);
    wait(&rv);
    assert(WEXITSTATUS(rv) == RETURN_CODE);
  }
  return EXIT_SUCCESS;
}