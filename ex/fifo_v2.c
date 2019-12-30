#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

void write_fifo(void);
void read_fifo(void);

#define NAMEDPIPE_NAME "/tmp/my_named_pipe"
#define BUFSIZE 50

char buffer[] = "asdfasdfasdfasd";

int main() {
  if (mkfifo(NAMEDPIPE_NAME, 0777)) {
    perror("mkfifo");
    abort();
  }

  switch (fork()) {
  case -1:
    perror("fork");
    exit(1);
  case 0:
    read_fifo();
    exit(EXIT_SUCCESS);
  default:
    write_fifo();
    wait(NULL);
  }
  remove(NAMEDPIPE_NAME);
  return EXIT_SUCCESS;
}

void write_fifo(void) {
  int fd;
  char buf[] = "asdfasdfasdfasd";

  if ((fd = open(NAMEDPIPE_NAME, O_WRONLY)) <= 0) {
    perror("open for write");
    exit(EXIT_FAILURE);
  }
  if (write(fd, buf, sizeof(buf)) != sizeof(buf)) {
    perror("write");
  }
  close(fd);
}

void read_fifo(void) {
  int fd, len;
  char buf[BUFSIZE];

  if ((fd = open(NAMEDPIPE_NAME, O_RDONLY)) <= 0) {
    perror("open for read");
    exit(EXIT_FAILURE);
  }

  while ((len = read(fd, buf, BUFSIZE - 1)) == 0) {
  }

  if ((len = read(fd, buf, BUFSIZE - 1)) < 0) {
    perror("read");
    close(fd);
    return;
  }

  assert(strcmp(buffer, buf) == 0);
}