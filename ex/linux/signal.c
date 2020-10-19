#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int err;

int happened_SIGINT = 0;

void exit_handler(int sig_code) {
  if (sig_code == SIGABRT && happened_SIGINT) {
    exit(EXIT_SUCCESS);
  } else if (sig_code == SIGINT) {
    happened_SIGINT = 1;
    abort();
  }
}

#define check_error(msg) \
  if (err) {             \
    perror(msg);         \
    abort();             \
  }

int main() {
  struct sigaction act;
  act.sa_handler = &exit_handler;

  err = sigaction(SIGINT, &act, NULL);
  check_error("fail: SIGINT");
  err = sigaction(SIGABRT, &act, NULL);
  check_error("fail: SIGABRT");

  err = kill(getpid(), SIGINT);
  check_error("fail: kill");

  abort();
  return EXIT_FAILURE;
}