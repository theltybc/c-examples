#include <pcap.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

void get_one_packet(pcap_t *dev);
void get_packet(pcap_t *dev);

char err_buff[PCAP_ERRBUF_SIZE];

#define BUFFER_SIZE 100000
char data_buffer[BUFFER_SIZE];

#define check_error(res, msg)                 \
  if (res == NULL) {                          \
    printf("Fail: " msg " - %s\n", err_buff); \
    exit(EXIT_FAILURE);                       \
  }

int main() {
  char *dev_name = pcap_lookupdev(err_buff);
  // char *dev_name = "wlx00117f74dbf1";
  check_error(dev_name, "get device");
  printf("auto select dev: %s\n", dev_name);

  // pcap_t *pcap_open_live(const char *device, int snaplen, int promisc, int to_ms, char *errbuf)
  pcap_t *dev = pcap_open_live(dev_name, BUFFER_SIZE, 1, 10, err_buff);
  check_error(dev, "open device");

  get_one_packet(dev);

  get_packet(dev);

  return EXIT_SUCCESS;
}

// void callback_function(u_char *arg, const struct pcap_pkthdr* pkthdr, const u_char* packet)
void callback_function(u_char *arg, const struct pcap_pkthdr *data, const u_char *packet) {
  assert(arg == NULL);
  printf("caplen = %u; len = %u\n", data->caplen, data->len);
  assert(data->caplen == data->len);
  for (unsigned i = 0; i < data->len; i++) {
    putc(packet[i], stdout);
  }
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
  for (unsigned i = 0; i < data.caplen; i++) {
    putc(packet[i], stdout);
  }
  putc('\n', stdout);
}