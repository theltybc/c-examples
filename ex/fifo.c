#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/stat.h>
#include <sys/wait.h>

#define FIFO_FILE "MYFIFO"

void server(void);
void client(void);

int main(void) {
  switch (fork()) {
  case -1:
    perror("fork");
    exit(EXIT_FAILURE);
  case 0:
    client();
    exit(EXIT_SUCCESS);
  default:
    server();
    wait(NULL);
  }
  unlink(FIFO_FILE);
  return EXIT_SUCCESS;
}

void server(void) {
  FILE *fp;
  char readbuf[80];

  /* Create the FIFO if it does not exist */
  umask(0);
  mknod(FIFO_FILE, S_IFIFO | 0666, 0);
  while (1) {
    fp = fopen(FIFO_FILE, "r");
    if (fp != NULL && fgets(readbuf, sizeof(readbuf), fp) != NULL) {
      printf("Received string: %s\n", readbuf);
      fclose(fp);
      return;
    }
    fclose(fp);
  }
}

void client(void) {
  FILE *fp;

  if ((fp = fopen(FIFO_FILE, "w")) == NULL) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }
  fputs("test string", fp);
  fclose(fp);
}