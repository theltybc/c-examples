#include <stdio.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <assert.h>

int main() {
  int sock_r;
  sock_r = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (sock_r < 0) {
    perror("error in socket");
    abort();
  }

  char buffer[65536]; //to receive data
  struct sockaddr saddr;
  int saddr_len = sizeof(saddr);

  while (1) {

    //Receive a network packet and copy in to buffer
    ssize_t buflen = recvfrom(sock_r, buffer, sizeof(buffer), 0, &saddr, (socklen_t *)&saddr_len);
    if (buflen < 0) {
      printf("error in reading recvfrom function\n");
      abort();
    }

    struct ethhdr *eth = (struct ethhdr *)(buffer);
    printf("\nEthernet Header\n");
    printf("\t|-Source Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n", eth->h_source[0], eth->h_source[1], eth->h_source[2], eth->h_source[3], eth->h_source[4], eth->h_source[5]);
    printf("\t|-Destination Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n", eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], eth->h_dest[3], eth->h_dest[4], eth->h_dest[5]);
    printf("\t|-Protocol : %d\n", eth->h_proto);
    if (ntohs(eth->h_proto) != ETH_P_IP) {
      continue;
    }

    struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));
    printf("\t|-Version : %d\n", ip->version);
    printf("\t|-Internet Header Length : %d DWORDS or %d Bytes\n", ip->ihl, (ip->ihl) * 4);
    printf("\t|-Type Of Service : %d\n", (unsigned int)ip->tos);
    printf("\t|-Total Length : %d Bytes\n", ntohs(ip->tot_len));
    printf("\t|-Identification : %d\n", ntohs(ip->id));
    printf("\t|-Time To Live : %d\n", (unsigned int)ip->ttl);
    printf("\t|-Protocol : %d\n", (unsigned int)ip->protocol);
    printf("\t|-Header Checksum : %d\n", ntohs(ip->check));
    printf("\t|-Source IP : %s\n", inet_ntoa(*((struct in_addr *)&ip->saddr)));
    printf("\t|-Destination IP : %s\n", inet_ntoa(*((struct in_addr *)&ip->daddr)));

    /* getting actual size of IP header*/
    unsigned iphdrlen = ip->ihl * 4;
    char *data = NULL;
    int remaining_data = 0;
    if (ip->protocol == IPPROTO_UDP) {
      /* getting pointer to udp header*/
      struct udphdr *udp = (struct udphdr *)(buffer + iphdrlen + sizeof(struct ethhdr));
      printf("\t|-Source Port : %d\n", ntohs(udp->source));
      printf("\t|-Destination Port : %d\n", ntohs(udp->dest));
      printf("\t|-UDP Length : %d\n", ntohs(udp->len));
      printf("\t|-UDP Checksum : %d\n", ntohs(udp->check));
      data = (buffer + iphdrlen + sizeof(struct ethhdr) + sizeof(struct udphdr));
      remaining_data = buflen - (iphdrlen + sizeof(struct ethhdr) + sizeof(struct udphdr));
    } else if (ip->protocol == IPPROTO_TCP) {
      struct tcphdr *tcp = (struct tcphdr *)(buffer + iphdrlen + sizeof(struct ethhdr));
      printf("\t|-Source Port : %d\n", ntohs(tcp->source));
      printf("\t|-Destination Port : %d\n", ntohs(tcp->dest));
      data = (buffer + iphdrlen + sizeof(struct ethhdr) + sizeof(struct tcphdr));
      remaining_data = buflen - (iphdrlen + sizeof(struct ethhdr) + sizeof(struct tcphdr));
    }

    for (int i = 0; i < remaining_data; i++) {
      if (i != 0 && i % 16 == 0) {
        printf("\n");
      }
      printf(" %.2x ", data[i]);
      // printf("%c", data[i]);
    }
    printf("\n");

    printf("\n");
  }
  return EXIT_SUCCESS;
}