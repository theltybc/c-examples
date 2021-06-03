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

int fd;

const char *fileName = "/dev/ttyS2";

int open_port(const char *fileName);
void setup_port(int fd);
void get_conf(int fd, struct termios *conf);
void set_conf(int fd, const struct termios *conf);
void spam_to_port(int fd);
void set_rts(int fd, int flag);

int main() {
  fd = open_port(fileName);
  tcflush(fd, TCIOFLUSH);
  setup_port(fd);
  spam_to_port(fd);
  close(fd);
  return EXIT_SUCCESS;
}

int open_port(const char *fileName) {
  fd = open(fileName, O_RDWR | O_NOCTTY | O_NDELAY);
  fcntl(fd, F_SETFL, 0);
  if (fd < 0) {
    goto error;
  }
  if (0 != fcntl(fd, F_SETFL, 0)) {
    goto error;
  }
  return fd;
error:
  perror("Opening");
  abort();
}

void setup_port(int fd) {
  struct termios conf;
  get_conf(fd, &conf);

  conf.c_iflag = (0 | IGNBRK | IUTF8) & ~(0 | IGNBRK | IXON | IXOFF | IXANY | ICRNL);
  conf.c_oflag = (0) & ~(0 | OPOST);
  conf.c_lflag = (0 | ISIG | IEXTEN) & ~(0 | ECHOE | ECHOK | ECHO | ECHOCTL | ECHOKE | ECHONL | ISIG | IEXTEN | ICANON);
  conf.c_cflag = (0 | CS8 | CLOCAL | CRTSCTS | HUPCL | CSIZE | CSTOPB | CREAD | CLOCAL) & ~(0 | PARODD | CSTOPB);

  conf.c_cc[VMIN] = 0;
  conf.c_cc[VTIME] = 0;

  conf.c_line = (0 | TIOCM_RTS) & ~(0);

  cfsetispeed(&conf, B115200);
  cfsetospeed(&conf, B115200);
  printf("speed IN = %u\n", cfgetispeed(&conf));
  printf("speed OUT = %u\n", cfgetospeed(&conf));

  set_conf(fd, &conf);
}

void get_conf(int fd, struct termios *conf) {
  if (tcgetattr(fd, conf) != 0) {
    perror("Get parameters failed");
    abort();
  }
}

void set_conf(int fd, const struct termios *conf) {
  if (tcsetattr(fd, TCSANOW, conf) != 0) {
    perror("Set parameters failed");
    abort();
  }
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

void set_rts(int fd, int flag) {
  unsigned status;
  if (ioctl(fd, TIOCMGET, &status)) {
    goto error;
  }

  if (flag) {
    status |= (TIOCM_RTS | TIOCM_DTR);
  } else {
    status &= ~(TIOCM_RTS | TIOCM_DTR);
  }

  if (ioctl(fd, TIOCMSET, &status)) {
    goto error;
  }

  return;

error : {
  perror("set rts failed");
  abort();
}
}
