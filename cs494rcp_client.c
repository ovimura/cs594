#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "packet.h"


//#define PORT 8080

int sockfd;

int main(int argc, char** argv)
{
  if (argc != 4)
  {
    printf("Usage: %s <server_ip> <port> <file>\n", argv[0]);
  } else {
    printf("%s %s %s %s\n", argv[0], argv[1], argv[2], argv[3]);
    char buffer[1024];
    char* hello = "Hello from client";
    struct sockaddr_in servaddr;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      perror("socket creation failed");
      exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "131.252.208.103",&servaddr.sin_addr.s_addr);
    struct Packet_SYN_C syn;
    syn.type = W;
    syn.pkt_len = sizeof(short);
	    printf("len: %ld\n",sizeof(struct Packet_SYN_C));
    strcpy(syn.filename, "test.jpg");
    printf("sizeof syn: %ld\n",sizeof(syn));

    int n, len;
    char *serialize = (char *)malloc(sizeof(syn));
//    memset(serialize, 0,1032);
    memcpy(serialize, (const char*)&syn, 1032);
//    memcpy(serialize, hello, 8);
    for (int i=0;i<1032;i++)
	    printf("%02x ",serialize[i]);
//    struct Packet_SYN_C *pkt = (struct Packet_SYN_C*)serialize;
//    printf("filename: %s\n", pkt->filename);
//    printf("size of packet: %ld\n", sizeof(syn));
    sendto(sockfd, (const char*)serialize, 1032, MSG_CONFIRM, (const struct sockaddr*) &servaddr, sizeof(servaddr));
//    printf("Hello message sent.\n");
//    printf("%d %s %ld %p %ld\n", sockfd, (const char*)hello, strlen(hello),(const struct sockaddr*)&servaddr, sizeof(servaddr));
    n = recvfrom(sockfd, (char*)buffer, 1024, MSG_WAITALL, (struct sockaddr*)&servaddr, &len);
    buffer[n] = '\0';
    printf("Server: %s\n", buffer);

    close(sockfd);
  }
  return 0;
}
