#include "packet.h"

long PORT = 8080;

int sockfd;

struct sockaddr_in servaddr, cliaddr;

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

char * serialize_r(struct Packet_SYN_ACK_C *p)
{
  char *r = (char*)malloc(sizeof(struct Packet_SYN_ACK_C));
  memcpy(r, (const char*)p, 1024);
  return r;
}



char *hello = "Hello from server";
struct Packet_SYN_ACK_C s_ack;
char *buf;// = serialize_r(&s_ack);

int handshake(struct sockaddr_in cliaddr)
{
  char buffer[2048];// = malloc(2048);
  int len, n;
  n = recvfrom(sockfd, (char *)buffer, 2048, MSG_WAITALL, (struct sockaddr*)&cliaddr, &len);
  buffer[n] = '\0';
  printf("n: %d\n", n);

  struct Packet_SYN_C *p = (struct Packet_SYN_C*)desirealize_s(buffer);

  printf("Client: |%d|, |%d|, |%s|\n", p->type, p->pkt_len, p->filename);

//  struct Packet_SYN_ACK_C syn_ack;
//  syn_ack.type = R;
//  syn_ack.file_size = 1234567890;
  s_ack.type = R;
  s_ack.file_size = 3333333;
  buf = serialize_r(&s_ack);

  for(int j=0;j<10; j++) {
    sendto(sockfd, (const char*)buf, sizeof(s_ack), MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);
    printf("Hello message sent.\n");      
  }
  
  close(sockfd);
  return 1;
}

int main(int argc, char** argv)
{
  if (argc != 2)
    printf("Usage: %s <port>\n", argv[0]);
  else if (argc == 2) {
    if(isNumber(argv[1]))
    {
      PORT = atol(argv[1]);
    }
  }
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
  return 0;
}
