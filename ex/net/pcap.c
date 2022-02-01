#include <pcap.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <netinet/ether.h>
#include <string.h>
#include <netdb.h>

struct package_info {
  char *ip_src;
  u_short port_src;
  char *host_src;
  char *ip_dst;
  u_short port_dst;
  char *host_dst;
  tcp_seq seq_num;
  tcp_seq ack_num;
};

char *reverse_dns_lookup(char *ip_addr);
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
  pcap_if_t *dev_list = NULL;
  if (pcap_findalldevs(&dev_list, err_buff)) {
    printf("Fail: pcap_findalldevs - %s\n", err_buff);
    return EXIT_FAILURE;
  }
  if (dev_list == NULL){
    printf("pcap_findalldevs not found devs\n");
    return EXIT_SUCCESS;
  }
  char dev_name[strlen(dev_list->name) + 1];
  strcpy(dev_name,dev_list->name);
  // pcap_if_t *dev_tmp = dev_list;
  // while (dev_tmp != NULL) {
  //   printf("dev: %s\n", dev_tmp->name);
  //   assert(dev_tmp != dev_list->next);
  //   dev_tmp = dev_list->next;
  // }
  pcap_freealldevs(dev_list);
  // char *dev_name = "wlx00117f74dbf1";
  check_error(dev_name, "get device");
  printf("auto select dev: %s\n", dev_name);

  // int pcap_lookupnet(const char *device, bpf_u_int32 *netp, bpf_u_int32 *maskp, char *errbuf)
  // u_int32_t ip, mask;
  struct in_addr ip, mask;
  res = pcap_lookupnet(dev_name, (u_int32_t *)&ip, (u_int32_t *)&mask, err_buff);
  if (res) {
    printf("Fail: lookupnet - %s\n", err_buff);
  } else {
    // printf("IP: %x; Mask: %x\n", ip, mask);
    printf("IP: %s; ", inet_ntoa(ip));
    printf("Mask: %s\n", inet_ntoa(mask));
  }

  // pcap_t *pcap_open_live(const char *device, int snaplen, int promisc, int to_ms, char *errbuf)
  pcap_t *dev = pcap_open_live(dev_name, BUFFER_SIZE, 1, 10, err_buff);
  check_error(dev, "open device");

  add_filter(dev);

  // get_one_packet(dev);

  while (1) {
    get_packet(dev);
  }

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

// Resolves the reverse lookup of the hostname
char *reverse_dns_lookup(char *ip_addr) {
  struct sockaddr_in temp_addr;
  socklen_t len;
  char buf[NI_MAXHOST];

  temp_addr.sin_family = AF_INET;
  temp_addr.sin_addr.s_addr = inet_addr(ip_addr);
  len = sizeof(struct sockaddr_in);

  if (getnameinfo((struct sockaddr *)&temp_addr, len, buf, sizeof(buf), NULL, 0, NI_NAMEREQD)) {
    printf("Could not resolve reverse lookup of hostname\n");
    return NULL;
  }
  return strdup(buf);
}

struct package_info *get_packet_info(const u_char *packet) {
  struct package_info *pi = malloc(sizeof(struct package_info));

  if (pi == NULL) {
    return NULL;
  }

  struct iphdr *ip = (struct iphdr *)(packet + ETHER_HDR_LEN);
  if (ip->protocol == IPPROTO_TCP) {  /* tcp protocol number */
    assert(ip->version == IPVERSION); // IP version 4
    struct tcphdr *tcp = (struct tcphdr *)(packet + ETHER_HDR_LEN + ip->ihl * 4);

    pi->port_src = ntohs(tcp->th_sport);
    pi->port_dst = ntohs(tcp->th_dport);

    struct servent *x = getservbyport(tcp->th_sport, NULL);
    if (x != NULL) {
      printf("service %s\n", x->s_name);
    }

    char *ip_addr = inet_ntoa(*(struct in_addr *)&(ip->saddr));
    pi->ip_src = strdup(ip_addr);
    pi->host_src = reverse_dns_lookup(ip_addr);

    ip_addr = inet_ntoa(*(struct in_addr *)&(ip->daddr));
    pi->ip_dst = strdup(ip_addr);
    pi->host_dst = reverse_dns_lookup(ip_addr);

    pi->seq_num = ntohl(tcp->th_seq);
    pi->ack_num = ntohl(tcp->th_ack);
  } else {
    return NULL;
  }

  return pi;
}

void free_packet_info(struct package_info *pi) {
  free(pi->ip_src);
  free(pi->host_src);
  free(pi->ip_dst);
  free(pi->host_dst);
  free(pi);
}

// void callback_function(u_char *arg, const struct pcap_pkthdr* pkthdr, const u_char* packet)
void callback_function(u_char *arg __attribute__((unused)), const struct pcap_pkthdr *pkthdr,
                       const u_char *packet __attribute__((unused))) {
  assert(arg == NULL);
  struct package_info *pi = get_packet_info(packet);
  if (pi != NULL) {
    printf("src address: %s:%u ", pi->ip_src, pi->port_src);
    printf("src host: %s \n", pi->host_src);
    printf("dst address: %s:%u ", pi->ip_dst, pi->port_dst);
    printf("dst host: %s \n", pi->host_dst);

    printf("seq number: %u ack number: %u \n", pi->seq_num, pi->ack_num);
    free_packet_info(pi);
  } else {
    printf("fail: get packet_info\n");
  }

  printf("caplen = %u; len = %u\n", pkthdr->caplen, pkthdr->len);
  assert(pkthdr->caplen == pkthdr->len);
  // print(packet, pkthdr->len);
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