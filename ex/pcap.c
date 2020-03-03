#include <pcap.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>

void add_filter(pcap_t *dev);
void get_one_packet(pcap_t *dev);
void get_packet(pcap_t *dev);
void print(const u_char *data, bpf_u_int32 len);

char err_buff[PCAP_ERRBUF_SIZE];

#define BUFFER_SIZE 100000
char data_buffer[BUFFER_SIZE];

#define check_error(res, msg)                 \
  if (res == NULL) {                          \
    printf("Fail: " msg " - %s\n", err_buff); \
    exit(EXIT_FAILURE);                       \
  }

int main() {
  int res;
  char *dev_name = pcap_lookupdev(err_buff);
  // char *dev_name = "wlx00117f74dbf1";
  check_error(dev_name, "get device");
  printf("auto select dev: %s\n", dev_name);

  // int pcap_lookupnet(const char *device, bpf_u_int32 *netp, bpf_u_int32 *maskp, char *errbuf)
  u_int32_t ip, mask;
  res = pcap_lookupnet(dev_name, &ip, &mask, err_buff);
  if (res) {
    printf("Fail: lookupnet - %s\n", err_buff);
  } else {
    printf("IP: %x; Mask: %x\n", ip, mask);
  }

  // pcap_t *pcap_open_live(const char *device, int snaplen, int promisc, int to_ms, char *errbuf)
  pcap_t *dev = pcap_open_live(dev_name, BUFFER_SIZE, 1, 10, err_buff);
  check_error(dev, "open device");

  add_filter(dev);

  get_one_packet(dev);

  get_packet(dev);

  return EXIT_SUCCESS;
}

void add_filter(pcap_t *dev) {
  // int pcap_compile(pcap_t *p, struct bpf_program *fp, const char *str, int optimize, bpf_u_int32 mask);
  int res;
  struct bpf_program fp;
  // res = pcap_compile(dev, &fp, "tcp", 1, 0xFFFFFF00); // mask 255.255.255.0
  res = pcap_compile(dev, &fp, "tcp", 1, PCAP_NETMASK_UNKNOWN);
  if (res) {
    pcap_perror(dev, err_buff);
    printf("Fail compile filter - %s\n", err_buff);
    exit(EXIT_FAILURE);
  }
  // int pcap_setfilter(pcap_t * p, struct bpf_program * fp)
  res = pcap_setfilter(dev, &fp);
  if (res) {
    pcap_perror(dev, err_buff);
    printf("Fail set filter - %s\n", err_buff);
    exit(EXIT_FAILURE);
  }
}

// void callback_function(u_char *arg, const struct pcap_pkthdr* pkthdr, const u_char* packet)
void callback_function(u_char *arg __attribute__((unused)), const struct pcap_pkthdr *pkthdr, const u_char *packet __attribute__((unused))) {
  assert(arg == NULL);
  printf("caplen = %u; len = %u\n", pkthdr->caplen, pkthdr->len);
  assert(pkthdr->caplen == pkthdr->len);
  print(packet, pkthdr->len);
  putc('\n', stdout);
}

void get_packet(pcap_t *dev) {
  const int count_package = 1;
  // int pcap_loop(pcap_t *p, int cnt, pcap_handler callback, u_char *user)
  int res = pcap_loop(dev, count_package, callback_function, NULL);
  if (res) {
    printf("fail: pcap_loop\n");
    exit(EXIT_FAILURE);
  }
}

void get_one_packet(pcap_t *dev) {
  struct pcap_pkthdr data;
  // const u_char *pcap_next(pcap_t *p, struct pcap_pkthdr *h)
  const u_char *packet = pcap_next(dev, &data);
  if (packet == NULL) {
    printf("Fail: pcap_next\n");
    exit(EXIT_FAILURE);
  }
  printf("caplen = %u; len = %u\n", data.caplen, data.len);
  assert(data.caplen == data.len);
  print(packet, data.caplen);
  putc('\n', stdout);
}

void print(const u_char *data, bpf_u_int32 len) {
  for (unsigned i = 0; i < len; i++) {
    if (isprint(data[i])) {
      putc(data[i], stdout);
    } else {
      // printf("0x%x", data[i]);
      putc('*', stdout);
    }
  }
}