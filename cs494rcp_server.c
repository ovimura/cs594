#include "packets.h"

long PORT = 1080;

int sockfd;
char *fname;


struct sockaddr_in servaddr, cliaddr;
struct Packet_SYN_C *p;

struct Packet_SYN_C *desirealize_s(char *b)
{
  p = (struct Packet_SYN_C*)b;
  return p;
}

struct Packet_ACK_C *pp;
struct Packet_ACK_C *desirealize_w(char *b)
{
  pp = (struct Packet_ACK_C*)b;
  return pp;
}

char r[1024];
char * serialize_r(struct Packet_SYN_ACK_C *ppp)
{
  memcpy(r, (const char*)ppp, 1024);
  return r;
}

struct Packet_SYN_ACK_C s_ack;
char *buf;
char bu[1024];
int len;

int handshake(struct sockaddr_in cliaddri)
{
  char buffer[1024];
  int n;
  
  n = recvfrom(sockfd, NULL, 0, MSG_PEEK | MSG_TRUNC, (struct sockaddr*)&cliaddr, &len);
  char *pkt = malloc(n*sizeof(char));
  recvfrom(sockfd, pkt, n, 0, (struct sockaddr*)(&cliaddr), &len);
  printf("receives syn packet, n: %d\n", n);
  
  struct Packet_SYN_C *p = (struct Packet_SYN_C*)desirealize_s(pkt);

  printf("Client: |%d|, |%d|, |%s|\n", p->type, p->pkt_len, p->filename);

  s_ack.type = R;
  s_ack.file_size = find_size(p->filename);
  fname = malloc(sizeof(p->filename));
  memcpy(fname, p->filename, sizeof(p->filename));
  buf = serialize_r(&s_ack);

  printf("\n\ns_ack: %ld\n", sizeof(s_ack));

  for(int j=0;j<1; j++) {
sendto(sockfd,(const char*)buf,100,MSG_CONFIRM,(const struct sockaddr *)&cliaddr,len);
    printf("\nsyn+ack packet message sent to server\n");
  }
  n = recvfrom(sockfd, NULL, 0, MSG_PEEK | MSG_TRUNC, (struct sockaddr*)&cliaddr, &len);
  char *pkt1 = malloc(n*sizeof(char));
  recvfrom(sockfd, pkt1, n, 0, (struct sockaddr*)(&cliaddr), &len);
  printf("ack connection packet received from client, n: %d\n",n);
  printf("type: %d\n", ((struct Packet_ACK_C*)pkt1)->type);
  return 1;
}


struct Packet_DATA_D data;
//char *fname;

int send_data()
{
  printf("start sending data\n");
  data.type = D;
  data.seq_num = 1;
  data.pkt_len = sizeof(struct Packet_DATA_D*);
  FILE *fd = fopen(fname,"rb");
//  FILE *fc = fopen("f.jpg.server","wb");
  if(fd==NULL)
  {
    printf("Unable to open file\n");
    return -2;
  }
  fseek(fd, 0, SEEK_END);
  long fsize = ftell(fd);
  fseek(fd, 0, SEEK_SET);
  printf("%ld\n",fsize);
  int n=-2;
  while ((n = fread(data.data, 1, 1024, fd))>0)
  {
    printf("r %d", n);
    arm_timer();
    sendto(sockfd, (const char*)data.data, n, MSG_CONFIRM, (const struct sockaddr*)&cliaddr, len);
    //disarm_timer();
//    n = recvfrom(sockfd, NULL, 0, MSG_PEEK | MSG_TRUNC, (struct sockaddr*)(&servaddr),&len);
    n=recvfrom(sockfd,NULL,0,MSG_PEEK|MSG_TRUNC,(struct sockaddr*)(&servaddr),&len);
    while((n<0 && getExpired()==0)||((n<0)&&getExpired()==1))
    {
      sendto(sockfd, (const char*)data.data, n, MSG_CONFIRM, (const struct sockaddr*)&cliaddr, len);
      n=recvfrom(sockfd,NULL,0,MSG_PEEK|MSG_TRUNC,(struct sockaddr*)(&servaddr),&len);
    }
    if(n>0 && getExpired()==0)
      disarm_timer();
    char *pkt = malloc(n*sizeof(char));
    struct Packet_ACK_D *t;
    recvfrom(sockfd, pkt, n, 0, (struct sockaddr*)(&servaddr), &len);
    if (n != -1)
    {
       t = (struct Packet_ACK_D*)pkt;
       printf("typeeeee :%d\n",t->type);
    }
//    if(fwrite(&data.data, 1, n, fc)>0)
//    {
//      printf("w\n");
//    }
  }
  printf("done\n");
  fclose(fd);
//  fclose(fc);
  return 0;
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
  set_signal();
  setup_one_shot_timer(1);
  send_data();
  return 0;
}
