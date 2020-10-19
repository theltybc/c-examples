#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <errno.h>
#include <assert.h>
#include <time.h>

#include "serial.h"

int dev_fd;
int file_fd;
FILE *error_log;
// #define error_log stderr

const char *dev_file = "/dev/ttyUSB0";
const char *log_file = "/home/we/gps_log.log";
const char *error_file = "/home/we/log";

char buffer[150];

void open_log(void);
void from_port_to_log(void);

int main() {
  int res;
  while (1) {
    error_log = fopen(error_file, "wa+");
    if (error_log == NULL) {
      perror("FAIL: error_log open");
      abort();
    }
    dev_fd = open_port(dev_file);
    if (dev_fd < 0) {
      return EXIT_FAILURE;
    }
    flush_port(dev_fd);
    res = setup_port(dev_fd);
    if (res != 0) {
      return EXIT_FAILURE;
    }
    open_log();
    from_port_to_log();
    close(dev_fd);
    close(file_fd);
    fclose(error_log);
    usleep(10);
  }

  return EXIT_SUCCESS;
}

void open_log(void) {
  file_fd = open(log_file, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
  if (file_fd <= 0) {
    fprintf(error_log, "fail open file - %s\n", log_file);
    abort();
  }
}

void from_port_to_log(void) {
  errno = 0;
  while (!errno) {
    size_t len = read(dev_fd, buffer, sizeof(buffer));
    // fprintf(error_log, "read %lu \n", len);
    if (len > 0) {
      if (buffer[0] == '\0') { // this can happen on startup
        fprintf(error_log, "zero length string\n");
        return;
      }
      size_t writed_len = write(file_fd, buffer, len);

      // fprintf(error_log, "write %lu char %d\n", writed_len, buffer[0]);
      if (writed_len != len) {
        fprintf(error_log, "not all data been write in file. errno = %i\n", errno);
      }
    }
  }
}
