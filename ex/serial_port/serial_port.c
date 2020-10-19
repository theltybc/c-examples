#include "serial.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>

int dev_fd;
int res;
// FILE *error_log;
#define error_log stderr

const char *dev_file = "/dev/ttyUSB0";

void spam_to_port(void);

int main() {
  // error_log = fopen("/home/pi/log", "a+");
  dev_fd = open_port(dev_file);
  if (dev_fd < 0) {
    return EXIT_FAILURE;
  }
  res = setup_port(dev_fd);
  if (res != 0) {
    return EXIT_FAILURE;
  }
  // setRS485();
  flush_port(dev_fd);
  spam_to_port();
  close(dev_fd);
  // fclose(error_log);

  return EXIT_SUCCESS;
}

void spam_to_port(void) {
  uint32_t i1 = 0;
  const char *str = "1234567890123456789123456789123456789123456789123456789"
                    "1234567890123456789123456789123456789123456789123456789";
  const size_t len = strlen(str);
  while (i1 < 10) {
    // set_rts(0);
    const size_t res = write(dev_fd, str, len);
    tcdrain(dev_fd);
    // set_rts(1);
    assert(res == len);

    usleep(200000);
    i1++;
  }
}
