#include <stdio.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <assert.h>

unsigned short checksum(unsigned short *buff, int _16bitword) {
  unsigned long sum;
  for (sum = 0; _16bitword > 0; _16bitword--)
    sum += htons(*(buff)++);
  sum = ((sum >> 16) + (sum & 0xFFFF));
  sum += (sum >> 16);
  return (unsigned short)(~sum);
}

int main() {
  int sock_raw = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW);
  if (sock_raw == -1) {
    perror("error in socket");
    abort();
  }
  struct ifreq ifreq_i;

  ifreq_i.ifr_ifindex = 2;
  if ((ioctl(sock_raw, SIOCGIFNAME, &ifreq_i)) < 0) {
    perror("ioctl reading name");
  }
  printf("interface = %s\n", ifreq_i.ifr_name);

  if ((ioctl(sock_raw, SIOCGIFINDEX, &ifreq_i)) < 0) {
    perror("error in index ioctl reading"); //getting Index Name
  }
  printf("index = %d\n", ifreq_i.ifr_ifindex);

  if ((ioctl(sock_raw, SIOCGIFHWADDR, &ifreq_i)) < 0) { //getting MAC Address
    perror("error in SIOCGIFHWADDR ioctl reading");
  }

  if (ioctl(sock_raw, SIOCGIFADDR, &ifreq_i) < 0) { //getting IP Address
    printf("error in SIOCGIFADDR \n");
  }
  assert(ifreq_i.ifr_addr.sa_family == PF_INET);
  printf("address = %s\n", inet_ntoa((*(struct sockaddr_in *)&ifreq_i.ifr_addr).sin_addr));

  unsigned char sendbuff[64];
  size_t total_len = 0;
  memset(sendbuff, 0, 64);

  struct ethhdr *eth = (struct ethhdr *)(sendbuff);

  eth->h_source[0] = ifreq_i.ifr_hwaddr.sa_data[0];
  eth->h_source[1] = ifreq_i.ifr_hwaddr.sa_data[1];
  eth->h_source[2] = ifreq_i.ifr_hwaddr.sa_data[2];
  eth->h_source[3] = ifreq_i.ifr_hwaddr.sa_data[3];
  eth->h_source[4] = ifreq_i.ifr_hwaddr.sa_data[4];
  eth->h_source[5] = ifreq_i.ifr_hwaddr.sa_data[5];

  /* filling destination mac. DESTMAC0 to DESTMAC5 are macro having octets of mac address. */
  eth->h_dest[0] = 0;
  eth->h_dest[1] = 0;
  eth->h_dest[2] = 0;
  eth->h_dest[3] = 0;
  eth->h_dest[4] = 0;
  eth->h_dest[5] = 0;

  eth->h_proto = htons(ETH_P_IP); //means next header will be IP header

  /* end of ethernet header */
  total_len += sizeof(struct ethhdr);

  struct iphdr *iph = (struct iphdr *)(sendbuff + sizeof(struct ethhdr));
  iph->ihl = 5;
  iph->version = IPVERSION;
  iph->tos = 16;
  iph->id = htons(10201);
  iph->ttl = 64;
  iph->protocol = IPPROTO_UDP;
  iph->saddr = inet_addr(inet_ntoa((((struct sockaddr_in *)&(ifreq_i.ifr_addr))->sin_addr)));
  iph->daddr = inet_addr("destination_ip"); // put destination IP address

  total_len += sizeof(struct iphdr);

  struct udphdr *uh = (struct udphdr *)(sendbuff + sizeof(struct iphdr) + sizeof(struct ethhdr));

  uh->source = htons(23451);
  uh->dest = htons(23452);
  uh->check = 0;

  total_len += sizeof(struct udphdr);

  sendbuff[total_len++] = 0xAA;
  sendbuff[total_len++] = 0xBB;
  sendbuff[total_len++] = 0xCC;
  sendbuff[total_len++] = 0xDD;
  sendbuff[total_len++] = 0xEE;

  uh->len = htons((total_len - sizeof(struct iphdr) - sizeof(struct ethhdr)));
  //UDP length field
  iph->tot_len = htons(total_len - sizeof(struct ethhdr));
  //IP length field

  iph->check = checksum((unsigned short *)(sendbuff + sizeof(struct ethhdr)), (sizeof(struct iphdr) / 2));

  assert(total_len < sizeof(sendbuff));

  struct sockaddr_ll sadr_ll;
  sadr_ll.sll_ifindex = ifreq_i.ifr_ifindex; // index of interface
  sadr_ll.sll_halen = ETH_ALEN;              // length of destination mac address
  sadr_ll.sll_addr[0] = 0;
  sadr_ll.sll_addr[1] = 0;
  sadr_ll.sll_addr[2] = 0;
  sadr_ll.sll_addr[3] = 0;
  sadr_ll.sll_addr[4] = 0;
  sadr_ll.sll_addr[5] = 0;

  ssize_t send_len = sendto(sock_raw, sendbuff, 64, 0, (const struct sockaddr *)&sadr_ll, sizeof(struct sockaddr_ll));
  if (send_len < 0) {
    printf("error in sending....sendlen = %ld....\n", send_len);
    perror("sand");
    abort();
  }

  return EXIT_SUCCESS;
}