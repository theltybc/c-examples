// #include <linux/serial_core.h>
// #include <linux/serial_reg.h>
#include <linux/serial.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <errno.h>
#include <assert.h>

int dev_fd;
// FILE *error_log;
#define error_log stderr

const char *dev_file = "/dev/ttyS2";

void open_port(void);
void flush_port(void);
void setup_port(void);
void get_conf(struct termios *conf);
void set_conf(const struct termios *conf);
void spam_to_port(void);
void setRS485(void);
void set_rts(int flag);

int main() {
  // error_log = fopen("/home/pi/log", "a+");
  open_port();
  // setup_port();
  setRS485();
  flush_port();
  spam_to_port();
  close(dev_fd);
  // fclose(error_log);

  return EXIT_SUCCESS;
}

void open_port(void) {
  dev_fd = open(dev_file, O_RDWR | O_NOCTTY | O_NDELAY);
  if (dev_fd < 0) {
    fprintf(error_log, "fail open port\n");
    abort();
  }
  int res = fcntl(dev_fd, F_SETFL, 0);
  if (res != 0) {
    fprintf(error_log, "fail fcntl\n");
  }
}

void flush_port(void) {
  int res = tcflush(dev_fd, TCIOFLUSH);
  if (res != 0) {
    fprintf(error_log, "fail tcflush\n");
  }
}

void setup_port(void) {
  struct termios conf;
  get_conf(&conf);

  conf.c_iflag =
      (0 | IGNBRK | IUTF8) & ~(0 | IXON | IXOFF | IXANY | ICRNL);
  conf.c_oflag = (0) & ~(0 | OPOST);
  conf.c_lflag =
      (0 | IEXTEN) &
      ~(0 | ECHOE | ECHOK | ECHO | ECHOCTL | ECHOKE | ECHONL | ISIG | IEXTEN | ICANON);
  conf.c_cflag =
      (0 | CS8 | CLOCAL | CRTSCTS | HUPCL | CSIZE | CREAD | CLOCAL) &
      ~(0 | PARODD | CSTOPB);

  conf.c_cc[VMIN] = 10;
  conf.c_cc[VTIME] = 1;

  conf.c_line = (0 | TIOCM_RTS) & ~(0);

  cfsetispeed(&conf, B9600);
  cfsetospeed(&conf, B9600);

  set_conf(&conf);
}

void get_conf(struct termios *conf) {
  if (tcgetattr(dev_fd, conf) != 0) {
    fprintf(error_log, "Get parameters failed %s %i\n", dev_file, errno);
    abort();
  }
}

void set_conf(const struct termios *conf) {
  if (tcsetattr(dev_fd, TCSANOW, conf) != 0) {
    fprintf(error_log, "Set parameters failed %s %i\n", dev_file, errno);
    abort();
  }
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

void setRS485(void) {
  struct serial_rs485 rs485conf;
  /* Enable RS485 mode: */
  rs485conf.flags = SER_RS485_ENABLED;

  //    /* Set logical level for RTS pin equal to 1 when sending: */
  // rs485conf.flags |= SER_RS485_RTS_ON_SEND;
  //    /* or, set logical level for RTS pin equal to 0 when sending: */
  rs485conf.flags &= ~(SER_RS485_RTS_ON_SEND);
  //
  //    /* Set logical level for RTS pin equal to 1 after sending: */
  rs485conf.flags |= SER_RS485_RTS_AFTER_SEND;
  //    /* or, set logical level for RTS pin equal to 0 after sending: */
  // rs485conf.flags &= ~(SER_RS485_RTS_AFTER_SEND);
  //
  //    /* Set this flag if you want to receive data even whilst sending data */
  // rs485conf.flags |= SER_RS485_RX_DURING_TX;
  rs485conf.delay_rts_after_send = 5;
  rs485conf.delay_rts_before_send = 5;

  if (ioctl(dev_fd, TIOCGRS485, &rs485conf) != 0) {
    // if (ioctl(dev_fd, TIOCSRS485, &rs485conf) != 0) {
    fprintf(error_log, "Setting the new parameters failed: errno %i\n", errno);
  }
}


void set_rts(int flag) {
  unsigned status;
  if (ioctl(dev_fd, TIOCMGET, &status)) {
    goto error;
  }

  if (flag) {
    status |= (TIOCM_RTS | TIOCM_DTR);
  } else {
    status &= ~(TIOCM_RTS | TIOCM_DTR);
  }

  if (ioctl(dev_fd, TIOCMSET, &status)) {
    goto error;
  }

  return;

error : {
  printf("set rts failed errno %i", errno);
  abort();
}
}