/**************************************************************************
 Excerpt from "Linux Programmer's Guide - Chapter 6"
 (C)opyright 1994-1995, Scott Burkett
 **************************************************************************
 MODULE: pipe.c
 **************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

int main(void) {
  int fd[2];
  size_t nbytes;
  pid_t childpid;
  char string[] = "Hello, world!\n";
  char readbuffer[80];

  assert(pipe(fd) == 0);
  if ((childpid = fork()) == -1) {
    perror("fork");
    exit(1);
  }
  if (childpid == 0) {
    close(fd[0]);
    sleep(1);
    nbytes = write(fd[1], string, strlen(string));
    assert(nbytes == strlen(string));
    exit(0);
  } else {
    close(fd[1]);
    // wait(NULL); // this can be remove
    nbytes = read(fd[0], readbuffer, sizeof(readbuffer));
    assert(nbytes == strlen(string));
    printf("Received string: %s\n", readbuffer);
  }
  return (0);
}