#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/ip_icmp.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <assert.h>

void __attribute__((unused)) print_bin(void *mem, size_t size) {
  char buff[9];
  size_t byte = 0;
  uint8_t bit;
  while (byte < size) {
    bit = 8;
    while (bit > 0) {
      bit--;
      if (((char *)mem)[byte] & (1 << bit)) {
        buff[7 - bit] = '1';
      } else {
        buff[7 - bit] = '0';
      }
    }
    buff[8] = '\0';
    printf(" %s", buff);
    byte++;
  }
}

int main() {
  int fd;

  fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (fd < 0) {
    perror("fail open socket");
    abort();
  }

  struct timeval tv_out;
  tv_out.tv_sec = 1;
  tv_out.tv_usec = 0;

  if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv_out, sizeof(tv_out))) {
    perror("fail set timeout send");
  }
  if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv_out, sizeof(tv_out))) {
    perror("fail set timeout receive");
  }

  struct icmphdr icmp;

  icmp.type = ICMP_ECHO;
  icmp.code = 0;
  icmp.checksum = 0;
  icmp.un.echo.id = getpid();
  icmp.un.echo.sequence = 0;
  while (1) {
    struct sockaddr_in addr;
    addr.sin_port = htons(7);
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, "192.168.0.130", &(addr.sin_addr));

    icmp.un.echo.sequence++;

    //  ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
    //                 const struct sockaddr *dest_addr, socklen_t addrlen);
    if (sendto(fd, &icmp, sizeof(struct icmphdr), MSG_CONFIRM, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) <= 0) {
      perror("fail write");
    }

    socklen_t len = sizeof(struct sockaddr_in);
    // int recvfrom(int s, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);
    char buff[1024];
    memset(buff, 0, sizeof(buff));
    struct icmphdr *icmp_rcv = (struct icmphdr *)(buff);
    icmp_rcv->type = ICMP_ECHO;
    icmp_rcv->code = 0;
    icmp_rcv->un.echo.id = getpid();
    size_t res = recvfrom(fd, &buff, sizeof(buff), MSG_WAITALL | MSG_PROXY, (struct sockaddr *)&addr, &len);
    if (res <= 0 && len == 0) {
      perror("fail receive");
    } else {
      printf("duff '");
      print_bin(buff, len);
      printf("'\n");
      // printf("addr '");
      // print_bin(&addr.sin_addr, sizeof(addr.sin_addr));
      // printf("'\n");

      printf("len: %u; ", len);
      printf("type: %u; ", icmp_rcv->type);
      printf("code: %u; ", icmp_rcv->code);
      printf("id: %u, %u; ", icmp_rcv->un.echo.id, getpid());
      printf("seq: %u, %u; ", icmp_rcv->un.echo.sequence, icmp.un.echo.sequence);
      printf("\n");
      // assert(icmp.un.echo.sequence == 1 || icmp_rcv->un.echo.sequence == icmp.un.echo.sequence);
      // assert(icmp_rcv->un.echo.id == getpid());
      // assert(icmp_rcv->type == ICMP_ECHOREPLY);
    }
    // return 0;
  }

  close(fd);
  return EXIT_SUCCESS;
}