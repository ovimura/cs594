#include "packet.h"

long PORT = 1080;

int sockfd;
char s[1032];
char * serialize_s(struct Packet_SYN_C *p)
{
  memcpy(s, (const char*)p, 1032);
  return s;
}

char m[1024];

char * serialize_w(struct Packet_ACK_C *p)
{
  memcpy(m, (const char*)p, 1024);
  return m;
}

//struct *Packet_SYN_C desirealize_s(char *b)
//{
//  return NULL;
//}


struct Packet_SYN_ACK_C *desirealize_r(char *b)
{
  struct Packet_SYN_ACK_C *p = (struct Packet_SYN_ACK_C*)b;
  return p;
}

struct sockaddr_in servaddr, cliaddr;

struct Packet_ACK_C ack_c;
struct Packet_SYN_C syn;

int handshake(struct sockaddr_in servaddr)
{
  char bb[1024]; 
  syn.type = S;
  syn.pkt_len = sizeof(syn);
  strcpy(syn.filename, "test.jpg");
  int n, len;
  sendto(sockfd, (const char*)serialize_s(&syn), 1024, MSG_CONFIRM, (const struct sockaddr*)&servaddr, sizeof(servaddr));
  printf("---\n");
  n = recvfrom(sockfd, (char*)bb, 1024, MSG_WAITALL, (struct sockaddr*)&servaddr, &len);

  printf("received: %d\n",n);
  struct Packet_SYN_ACK_C *p = desirealize_r(bb);
  printf("Server: %d %ld\n", p->type, p->file_size);
  
  ack_c.type = W;
  printf("send again\n"); 
  sendto(sockfd, (const char*)serialize_w(&ack_c), 1024, MSG_CONFIRM, (const struct sockaddr*)&servaddr, sizeof(servaddr));
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
    if(!isNumber(argv[2])){
	printf("port is not a number\n");
	exit(-1);
    } else {
      PORT = atol(argv[2]);
    }
    if(!validateFilePath(argv[3])) {
      fprintf(stderr, "the file provided is not valid\n");
      usage(argv[0]);
      exit(1);
    }
    char buffer[1024];
    char* hello = "Hello from client";

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

//    if(bind(sockfd, (const struct sockaddr*)&cliaddr, sizeof(cliaddr))<0)
//    {
//      perror("bind failed");
//      exit(EXIT_FAILURE);
//    }
    handshake(servaddr);
  }
  return 0;
}
