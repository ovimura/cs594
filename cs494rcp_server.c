/*#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h> */
#include "packet.h"

long PORT = 8080;

int sockfd;

struct Request {
  int sockfd;
  struct sockaddr_in cliaddr;
  int len;
  char* hello;
  char buf[1024];
};

struct Packet_SYN_C *desirealize_s(char *b)
{
  struct Packet_SYN_C *p = (struct Packet_SYN_C*)b;
  return p;
}

int handshake(struct sockaddr_in cliaddr)
{
  char *buffer = malloc(2048);
  int len, n;
  n = recvfrom(sockfd, (char *)buffer, 2048, MSG_WAITALL, (struct sockaddr*)&cliaddr, &len);
  buffer[n] = '\0';
  printf("n: %d\n", n);
  for (int i=0; i<2048;i++)
          printf("%02X ", buffer[i]);

  struct Packet_SYN_C *p = (struct Packet_SYN_C*)desirealize_s(buffer);

  printf("\nfilename: %s\n", p->filename);
  printf("type: %d\n", p->type);
  printf("pkt_len: %d\n", p->pkt_len);
  printf("Client: |%d|, |%d|, |%s|\n", ((struct Packet_SYN_C*)buffer)->type, ((struct Packet_SYN_C*)buffer)->pkt_len, ((struct Packet_SYN_C*)buffer)->filename);
  free(buffer);
  char*hello = "Hello from serveR";
  sendto(sockfd, (const char *)hello, 1024, MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);
  printf("Hello message sent.\n");      
  return 1;
}

int main(int argc, char** argv)
{

//  int sockfd;
//  char *buffer = malloc(2048);
  char *hello = "Hello from server";
  struct sockaddr_in servaddr, cliaddr;

  if (argc != 2)
    printf("Usage: %s <port>\n", argv[0]);
  else if (argc == 2) {
    if(isNumber(argv[1]))
    {
      PORT = atol(argv[1]);
    }
  }
  printf("port: %ld\n",PORT);
  // Create socket file descriptor
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0){
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));

  // Filling server information
  servaddr.sin_family = AF_INET; // IPv4
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(PORT);

  // Bind the socket with the server address
  if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
  {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }
  handshake(cliaddr);
/*  int len, n;
  n = recvfrom(sockfd, (char *)buffer, 2048, MSG_WAITALL, (struct sockaddr*)&cliaddr, &len);
  buffer[n] = '\0';
  printf("n: %d\n", n);
  for (int i=0; i<2048;i++)
	  printf("%02X ", buffer[i]);

  struct Packet_SYN_C *p = (struct Packet_SYN_C*)desirealize_s(buffer);

  printf("\nfilename: %s\n", p->filename);
  printf("type: %d\n", p->type);
  printf("pkt_len: %d\n", p->pkt_len);
  printf("Client: |%d|, |%d|, |%s|\n", ((struct Packet_SYN_C*)buffer)->type, ((struct Packet_SYN_C*)buffer)->pkt_len, ((struct Packet_SYN_C*)buffer)->filename);
  free(buffer);
  sendto(sockfd, (const char *)hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);
  printf("Hello message sent.\n");
*/
  return 0;
}
