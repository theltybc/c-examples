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

void open_port(void);
void setup_port(void);
void get_conf(struct termios *conf);
void set_conf(const struct termios *conf);
void spam_to_port(void);
void set_rts(int flag);

int main() {
  open_port();
  tcflush(fd, TCIOFLUSH);
  setup_port();
  spam_to_port();
  close(fd);
  return EXIT_SUCCESS;
}

void open_port(void) {
  fd = open(fileName, O_RDWR | O_NOCTTY | O_NDELAY);
  fcntl(fd, F_SETFL, 0);
  if (fd < 0) {
    printf("Opening ERROR %s\n", fileName);
    abort();
  } else {
    printf("Opening OK %s\n", fileName);
  }
  fcntl(fd, F_SETFL, 0);
}

void setup_port(void) {
  struct termios conf;
  get_conf(&conf);

  conf.c_iflag =
      (0 | IGNBRK | IUTF8) & ~(0 | IGNBRK | IXON | IXOFF | IXANY | ICRNL);
  conf.c_oflag = (0) & ~(0 | OPOST);
  conf.c_lflag =
      (0 | ISIG | IEXTEN) &
      ~(0 | ECHOE | ECHOK | ECHO | ECHOCTL | ECHOKE | ECHONL | ISIG | IEXTEN | ICANON);
  conf.c_cflag =
      (0 | CS8 | CLOCAL | CRTSCTS | HUPCL | CSIZE | CSTOPB | CREAD | CLOCAL) &
      ~(0 | PARODD | CSTOPB);

  conf.c_cc[VMIN] = 0;
  conf.c_cc[VTIME] = 0;

  conf.c_line = (0 | TIOCM_RTS) & ~(0);

  cfsetispeed(&conf, B115200);
  cfsetospeed(&conf, B115200);
  printf("speed IN = %u\n", cfgetispeed(&conf));
  printf("speed OUT = %u\n", cfgetospeed(&conf));

  set_conf(&conf);
}

void get_conf(struct termios *conf) {
  if (tcgetattr(fd, conf) != 0) {
    printf("Get parameters failed %s %i\n", fileName, errno);
    abort();
  }
}

void set_conf(const struct termios *conf) {
  if (tcsetattr(fd, TCSANOW, conf) != 0) {
    printf("Set parameters failed %s %i\n", fileName, errno);
    abort();
  } else {
    printf("Set parameters OK %s\n", fileName);
  }
}

void spam_to_port(void) {
  uint32_t i1 = 0;
  const char *str = "1234567890123456789123456789123456789123456789123456789"
                    "1234567890123456789123456789123456789123456789123456789";
  const size_t len = strlen(str);
  while (i1 < 10) {
    set_rts(1);
    const size_t res = write(fd, str, len);
    assert(res == len);
    tcdrain(fd);
    set_rts(0);
    usleep(200000);
    i1++;
  }
}

void set_rts(int flag) {
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
  printf("set rts failed code %i", errno);
  abort();
}
}
