#include "packets.h"

long PORT = 1080;

int sockfd;
char *fname;


struct sockaddr_in servaddr, cliaddr;

struct Packet_SYN_ACK_C s_ack;
char *buf;
char bu[1024];
int len;

int handshake(struct sockaddr_in cliaddri)
{
  int n;
  n = recvfrom(sockfd, NULL, 0, MSG_PEEK | MSG_TRUNC, (struct sockaddr*)&cliaddr, &len);
  char *pkt = malloc(n*sizeof(char));
  recvfrom(sockfd, pkt, n, 0, (struct sockaddr*)(&cliaddr), &len);
  
  struct Packet_SYN_C *p = (struct Packet_SYN_C*)desirealize_s(pkt);
  free(pkt);

  printf("Client: |%d|, |%d|, |%s|\n", p->type, p->pkt_len, p->filename);
  printf("filename: %s\n", p->filename); 

  s_ack.type = R;
  s_ack.file_size = find_size(p->filename);
  fname = malloc(sizeof(p->filename));
  memcpy(fname, p->filename, sizeof(p->filename));
  buf = serialize_r(&s_ack);
  memcpy(bu, buf, 1024);
  free(p);
  for(int j=0;j<1; j++) {
    sendto(sockfd,(const char*)bu,1024,0,(const struct sockaddr *)&cliaddr,sizeof(cliaddr));
    printf("\nsyn+ack packet message sent to server\n");
  }
  n = recvfrom(sockfd, NULL, 0, MSG_PEEK | MSG_TRUNC, (struct sockaddr*)&cliaddr, &len);
  printf("bbefore\n");
  char *pkt1 = malloc(n*sizeof(char));
  recvfrom(sockfd, pkt1, n, 0, (struct sockaddr*)(&cliaddr), &len);
  printf("ack connection packet received from client, n: %d\n",n);
  printf("type: %d\n", ((struct Packet_ACK_C*)pkt1)->type);
  return 1;
}

struct Packet_DATA_D data;

int send_data()
{
  printf("start sending data\n");
  data.type = D;
  printf("d: %d\n", data.type);
  data.seq_num = 0;
  data.pkt_len = 0; //sizeof(struct Packet_DATA_D);
  FILE *fd = fopen(fname,"rb");
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
    data.seq_num += 1;
    data.pkt_len = n;
    char *dt = serialize_d(&data);
    printf("r %d", n);
    arm_timer();
    // (const char*)data.data, n
    sendto(sockfd, dt, sizeof(struct Packet_DATA_D) , MSG_CONFIRM, (const struct sockaddr*)&cliaddr, len);

    n=recvfrom(sockfd,NULL,0,MSG_PEEK|MSG_TRUNC,(struct sockaddr*)(&servaddr),&len);
    while((n<0 && getExpired()==0)||((n<0)&&getExpired()==1))
    {
      sendto(sockfd, dt, sizeof(struct Packet_DATA_D), MSG_CONFIRM, (const struct sockaddr*)&cliaddr, len);
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
       printf("\ntypeeeee :%d\n",t->type);
    }
  }
  printf("done\n");
  fclose(fd);
  return 0;
}

struct Packet_ACK_CC a_cc;
char temp[1024];

void close_connection()
{
   a_cc.type = C;
   memcpy(temp, (struct Packet_ACK_CC *)&a_cc, 100);
   for (int i=0; i<1; i++)
     sendto(sockfd,(const char*)temp,100,0,(const struct sockaddr*)&cliaddr, sizeof(cliaddr));

   int n, len;
   printf("sent-----\n");
   n = recvfrom(sockfd, NULL, 0, MSG_PEEK|MSG_TRUNC, (struct sockaddr*)(&servaddr),&len);
   char *pkt = malloc(n*sizeof(char));
   printf("%d \n", n);
   recvfrom(sockfd, pkt, n, 0, (struct sockaddr*)(&servaddr),&len);
   struct Packet_ACK_CC *cc = (struct Packet_ACK_CC *)pkt;
   printf("--: > %d\n",cc->type);
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
  close_connection();
  return 0;
}
