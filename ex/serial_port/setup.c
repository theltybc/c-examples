#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/serial_reg.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include "serial.h"

int fd;

const char *fileName = "/dev/ttyS2";

void spam_to_port(int fd);

int main() {
  fd = open_port(fileName);
  if (fd < 0) {
    abort();
  }
  tcflush(fd, TCIOFLUSH);
  setup_port(fd);
  spam_to_port(fd);
  close(fd);
  return EXIT_SUCCESS;
}


void spam_to_port(int fd) {
  uint32_t i1 = 0;
  const char *str = "1234567890123456789123456789123456789123456789123456789"
                    "1234567890123456789123456789123456789123456789123456789";
  const size_t len = strlen(str);
  while (i1 < 10) {
    set_rts(fd, 1);
    const size_t res = write(fd, str, len);
    assert(res == len);
    tcdrain(fd);
    set_rts(fd, 0);
    usleep(200000);
    i1++;
  }
}

