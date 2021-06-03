#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/serial.h>
#include <termios.h>
#include <unistd.h>

int open_port(const char *dev_file);
int setup_port(int dev_fd);
int setRS485(int dev_fd);
int get_conf(int dev_fd, struct termios *conf);
int set_conf(int dev_fd, const struct termios *conf);
int flush_port(int dev_fd);
int set_rts(int dev_fd, int flag);

int open_port(const char *dev_file) {
  if (access(dev_file, F_OK | R_OK | W_OK) != 0) {
    perror("fail: dev access");
    return -1;
  }
  int dev_fd = open(dev_file, O_RDWR | O_NOCTTY /* | O_NDELAY */);
  if (dev_fd < 0) {
    perror("fail: open dev");
    return -1;
  }
  if (fcntl(dev_fd, F_SETFL, 0) != 0) {
    perror("fail: fcntl");
    return -1;
  }
  return dev_fd;
}

int setup_port(int dev_fd) {
  struct termios conf;

  if (get_conf(dev_fd, &conf)) {
    return -1;
  }

  conf.c_iflag = (0 | IGNBRK | IUTF8) & ~(0 | IXON | IXOFF | IXANY | ICRNL);
  conf.c_oflag = (0) & ~(0 | OPOST);
  conf.c_lflag = (0 | IEXTEN) & ~(0 | ECHOE | ECHOK | ECHO | ECHOCTL | ECHOKE | ECHONL | ISIG | IEXTEN | ICANON);
  conf.c_cflag = (0 | CS8 | CLOCAL | CRTSCTS | HUPCL | CSIZE | CREAD | CLOCAL) & ~(0 | PARODD | CSTOPB);

  conf.c_cc[VMIN] = 10;
  conf.c_cc[VTIME] = 1;

  conf.c_line = (0 | TIOCM_RTS) & ~(0);

  cfsetispeed(&conf, B9600);
  cfsetospeed(&conf, B9600);

  if (set_conf(dev_fd, &conf)) {
    return -1;
  }
  return 0;
}

int setRS485(int dev_fd) {
  struct serial_rs485 rs485conf;
  /* Enable RS485 mode: */
  rs485conf.flags = SER_RS485_ENABLED;

  //    /* Set logical level for RTS pin equal to 1 when sending: */
  // rs485conf.flags |= SER_RS485_RTS_ON_SEND;

  //    /* Set logical level for RTS pin equal to 1 after sending: */
  rs485conf.flags |= SER_RS485_RTS_AFTER_SEND;

  //    /* Set this flag if you want to receive data even whilst sending data */
  // rs485conf.flags |= SER_RS485_RX_DURING_TX;

  rs485conf.delay_rts_after_send = 5;
  rs485conf.delay_rts_before_send = 5;

  if (ioctl(dev_fd, TIOCGRS485, &rs485conf) != 0) {
    // if (ioctl(dev_fd, TIOCSRS485, &rs485conf) != 0) {
    perror("fail: setup rs485");
    return -1;
  }
  return 0;
}

int get_conf(int dev_fd, struct termios *conf) {
  if (tcgetattr(dev_fd, conf) != 0) {
    perror("fail: get config");
    return -1;
  }
  return 0;
}

int set_conf(int dev_fd, const struct termios *conf) {
  if (tcsetattr(dev_fd, TCSANOW, conf) != 0) {
    perror("fail: set config");
    return -1;
  }
  return 0;
}

int flush_port(int dev_fd) {
  int res = tcflush(dev_fd, TCIOFLUSH);
  if (res != 0) {
    perror("fail: flush\n");
    return -1;
  }
  return 0;
}

int set_rts(int dev_fd, int flag) {
  // todo: this can be replae by TIOCMBIS TIOCMBIC ?
  unsigned status;
  if (ioctl(dev_fd, TIOCMGET, &status)) {
    perror("fail: get rts status");
    return -1;
  }

  if (flag) {
    status |= (TIOCM_RTS | TIOCM_DTR);
  } else {
    status &= ~(TIOCM_RTS | TIOCM_DTR);
  }

  if (ioctl(dev_fd, TIOCMSET, &status)) {
    perror("fail: set rts status");
    return -1;
  }

  return 0;
}
