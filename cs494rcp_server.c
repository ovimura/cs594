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
  
  s_ack.type = R;
  s_ack.file_size = find_size(p->filename);
  fname = malloc(sizeof(p->filename));
  memcpy(fname, p->filename, sizeof(p->filename));
  free(p);
  buf = serialize_r(&s_ack);
  memcpy(bu, buf, sizeof(struct Packet_SYN_ACK_C));
  free(p);
  free(buf);
  sendto(sockfd,(const char*)bu,sizeof(struct Packet_SYN_ACK_C),0,(const struct sockaddr *)&cliaddr,sizeof(cliaddr));
  n = recvfrom(sockfd, NULL, 0, MSG_PEEK | MSG_TRUNC, (struct sockaddr*)&cliaddr, &len);
  char *pkt1 = malloc(n*sizeof(char));
  recvfrom(sockfd, pkt1, n, 0, (struct sockaddr*)(&cliaddr), &len);
  free(pkt1);
  return 1;
}

struct Packet_DATA_D data;

int send_data()
{ struct Packet_ACK_D *pkt_ack_d;
  data.type = D;
  data.seq_num = -1;
  data.pkt_len = 0; 

  FILE *fd = fopen(fname,"rb");
  if(fd==NULL)
  {
    printf("Unable to open file\n");
    return -2;
  }
  fseek(fd, 0, SEEK_END);
  long fsize = ftell(fd);
  fseek(fd, 0, SEEK_SET);
  int n=-2;
  while ((n = fread(data.data, 1, 1024, fd))>0)
  {
    data.seq_num += 1;
    data.pkt_len = n;
    char *dt = serialize_d(&data);
    do {
    sendto(sockfd, dt, sizeof(struct Packet_DATA_D), MSG_CONFIRM, (const struct sockaddr*)&cliaddr, len);
    arm_timer();
    n=recvfrom(sockfd,NULL,0,MSG_PEEK|MSG_TRUNC,(struct sockaddr*)(&servaddr),&len);
    while((n<0 && getExpired()==0)||((n<0)&&getExpired()==1))
    {
      sendto(sockfd, dt, sizeof(struct Packet_DATA_D), MSG_CONFIRM, (const struct sockaddr*)&cliaddr, len);
      printf(".");
      n=recvfrom(sockfd,NULL,0,MSG_PEEK|MSG_TRUNC,(struct sockaddr*)(&servaddr),&len);
    }
    free(dt);
    if(n>0 && getExpired()==0)
    {
      disarm_timer();
    }
    char *pkt = malloc(n*sizeof(char));
    recvfrom(sockfd, pkt, n, 0, (struct sockaddr*)(&servaddr), &len);

    if (n != -1)
    {
       pkt_ack_d = desirealize_ad(pkt);
    }
    free(pkt);
    } while(pkt_ack_d->seq_num != data.seq_num);
    free(pkt_ack_d);
  }
  fclose(fd);
  return 0;
}

struct Packet_ACK_CC a_cc;
char temp[1024];
struct Packet_ACK_CC *cc;
void close_connection()
{
   a_cc.type = C;
   char *acc = serialize_cc(&a_cc);
   sendto(sockfd,acc,sizeof(struct Packet_ACK_CC),0,(const struct sockaddr*)&cliaddr, sizeof(cliaddr));
   free(acc);
   int n, len;
   n = recvfrom(sockfd, NULL, 0, MSG_PEEK|MSG_TRUNC, (struct sockaddr*)(&servaddr),&len);
   char *pk = malloc(n*sizeof(char));
   recvfrom(sockfd, pk, n, 0, (struct sockaddr*)(&servaddr),&len);
   if(n>0)
     cc = desirealize_cc(pk);
   free(pk);
   if(close(sockfd) !=0)
     fprintf(stderr, "failed to close the server socket\n");
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
  setup_one_shot_timer(2);
  send_data();
  close_connection();
  return 0;
}
