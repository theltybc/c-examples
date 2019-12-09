// #include <linux/serial.h>
// #include <linux/serial_core.h>
// #include <linux/serial_reg.h>
// #include <stdint.h>
// #include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <errno.h>
#include <assert.h>

int dev_fd;
int file_fd;
FILE *error_log;
// #define error_log stderr

const char *dev_file = "/dev/ttyS2";
const char *log_file = "/home/pi/gps_log.log";

char buffer[150];

void open_port(void);
void flush_port(void);
void setup_port(void);
void get_conf(struct termios *conf);
void set_conf(const struct termios *conf);
void open_log(void);
void from_port_to_log(void);

int main() {
  while (1) {
    error_log = fopen("/home/pi/log", "a+");
    open_port();
    flush_port();
    setup_port();
    open_log();
    from_port_to_log();
    close(dev_fd);
    close(file_fd);
    fclose(error_log);
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

void open_port(void) {
  dev_fd = open(dev_file, O_RDWR | O_NOCTTY /* | O_NDELAY */);
  if (dev_fd < 0) {
    fprintf(error_log, "Opening ERROR %s\n", dev_file);
    abort();
  } else {
    // printf("Opening OK %s\n", dev_file);
  }
  int res = fcntl(dev_fd, F_SETFL, 0);
  if (res != 0){
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
