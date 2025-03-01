#include "packet.h"

long PORT = 1080;

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

struct Packet_ACK_C *desirealize_w(char *b)
{
  struct Packet_ACK_C *p = (struct Packet_ACK_C*)b;
  return p;
}
char r[1024];
char * serialize_r(struct Packet_SYN_ACK_C *p)
{
  //r = (char*)malloc(sizeof(struct Packet_SYN_ACK_C));
  memcpy(r, (const char*)p, 1024);
  return r;
}

struct Packet_SYN_ACK_C s_ack;
char *buf;// = serialize_r(&s_ack);

char bu[1024];

int handshake(struct sockaddr_in cliaddri)
{
  char buffer[1024];// = malloc(2048);
  int len, n;
  n = recvfrom(sockfd, (char *)buffer, 1024, MSG_WAITALL, (struct sockaddr*)&cliaddr, &len);
  buffer[n] = '\0';
  printf("receives syn packet, n: %d\n", n);

  struct Packet_SYN_C *p = (struct Packet_SYN_C*)desirealize_s(buffer);

  printf("Client: |%d|, |%d|, |%s|\n", p->type, p->pkt_len, p->filename);

  s_ack.type = R;
  s_ack.file_size = 3333333;
//  char a[sizeof(long)+sizeof(s_ack.type)];
//  memcpy(a, &s_ack.type, sizeof(s_ack.type));
//  memcpy(a, &s_ack.file_size, sizeof(long));
//  for (int ii=0;ii<sizeof(a);ii++)
//  {
//    printf("%02x ", a[ii]);
//  }
//  printf("\nsize of a ****: %ld\n", sizeof(a));
  buf = serialize_r(&s_ack);
//  memset(buf, 1, 1024);
  for(int i=0; i<100;i++) {
    printf("%02x ", buf[i]);
  }

  printf("\n\ns_ack: %ld\n", sizeof(s_ack));

  for(int j=0;j<1; j++) {
    sendto(sockfd, (const char*)buf, 1024, MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);
    printf("\nsyn+ack packet message sent to server\n");
  }

  n = recvfrom(sockfd, (char*)bu, 1024, MSG_WAITALL, (struct sockaddr*)&cliaddr, &len);
  bu[n] = '\0';
  printf("ack connection packet received from client, n: %d\n",n);
  printf("type: %d\n", ((struct Packet_ACK_C*)bu)->type);
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
