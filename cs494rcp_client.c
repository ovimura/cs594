/*#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>*/
#include "packet.h"


//#define PORT 8080

long PORT = 8080;

int sockfd;

char * serialize_s(struct Packet_SYN_C *p)
{
  char *s = (char*)malloc(sizeof(struct Packet_SYN_C));
  memcpy(s, (const char*)p, 1032);
  return s;
}

//struct *Packet_SYN_C desirealize_s(char *b)
//{
//  return NULL;
//}

int isServerIpFormat(char *ip)
{ char delim[] = ".";
  char *ptr = strtok(ip, delim);
  int i = 0; 
  while(ptr != NULL)
  {
    printf("%s \n", ptr);
    ptr = strtok(NULL, delim);
    i++;
  }
  printf("%d\n",i);
  if(i==4)
    return 1;
  else
    return 0;
}

int validateFileName(char *fname)
{
  return 1;
}

int handshake(struct sockaddr_in servaddr)
{
  struct Packet_SYN_C syn;
  char buffer[1024];
  syn.type = S;
  syn.pkt_len = sizeof(syn);
  strcpy(syn.filename, "test.jpg");
  int n, len;
//  char *ser = (char*)malloc(sizeof(syn));
//  memcpy(ser, (const char*)&syn, 1032);
  sendto(sockfd, (const char*)serialize_s(&syn), 1024, MSG_CONFIRM, (const struct sockaddr*)&servaddr, sizeof(servaddr));
  n = recvfrom(sockfd, (char*)buffer, 1024, MSG_WAITALL, (struct sockaddr*)&servaddr, &len);
  buffer[n] = '\0';
  printf("received: %d\n",n);
  printf("Server: %s\n", buffer);
  close(sockfd);

  return 1;
}

int main(int argc, char** argv)
{
  if (argc != 4)
  {
    printf("Usage: %s <server_ip> <port> <file>\n", argv[0]);
  } else {
    if(!isServerIpFormat(argv[1])) {
	    printf("server_ip is not correct format\n");
	    exit(-1);
    }
    if(isNumber(argv[2]))
	printf("port is a number\n");
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

/*    struct Packet_SYN_C syn;
    syn.type = W;
    syn.pkt_len = sizeof(struct Packet_SYN_C);
	    printf("len: %ld\n",sizeof(struct Packet_SYN_C));
    strcpy(syn.filename, "test.jpg");
    printf("sizeof syn: %ld\n",sizeof(syn));

    int n, len;
    char *serialize = (char *)malloc(sizeof(syn));
    memcpy(serialize, (const char*)&syn, 1032);
    for (int i=0;i<1032;i++)
      printf("%02x ",serialize[i]);
    sendto(sockfd, (const char*)serialize, 1024, MSG_CONFIRM, (const struct sockaddr*) &servaddr, sizeof(servaddr));
    n = recvfrom(sockfd, (char*)buffer, 1024, MSG_WAITALL, (struct sockaddr*)&servaddr, &len);
    buffer[n] = '\0';
    printf("Server: %s\n", buffer);

    close(sockfd); */
    handshake(servaddr);
  }
  return 0;
}
