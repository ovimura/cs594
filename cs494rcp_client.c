#include "packets.h"

long PORT = 1080;
char *fname;
int sockfd;
long file_size=0;


struct sockaddr_in servaddr, cliaddr;
struct Packet_ACK_C ack_c;
struct Packet_SYN_C syn;
char *bname;
char pkt[1024];


int handshake(struct sockaddr_in servaddr)
{
  syn.type = S;
  syn.pkt_len = sizeof(syn);
  char *dup = strdup(fname);
  bname = basename(dup);
  strcpy(syn.filename, fname);
  int n, len;
  char *ser = serialize_s(&syn);
  sendto(sockfd,ser,1024,MSG_PEEK|MSG_TRUNC,(const struct sockaddr*)&servaddr,sizeof(servaddr));

  n = recvfrom(sockfd, NULL, 0, MSG_PEEK | MSG_TRUNC, (struct sockaddr*)(&servaddr),&len);
  printf("==%d===\n", n);
  //Read packet
  recvfrom(sockfd, pkt, n, 0, (struct sockaddr*)(&servaddr),&len);
  
  struct Packet_SYN_ACK_C *p = desirealize_r(pkt);
  printf("Server SYN ACK Packet: %d %ld\n", p->type, p->file_size);
  file_size = p->file_size;

  ack_c.type = W;
  sendto(sockfd,(const char*)serialize_w(&ack_c),1024,MSG_PEEK|MSG_TRUNC,(const struct sockaddr*)&servaddr,sizeof(servaddr));
  return 1;
}

struct Packet_ACK_D pad;

int receive_data()
{
  int len, n;
  char pkt[1024];
  char *buffer;
  int size=0;
  char *temp;
  char *a;
  int b=0;
  char *tmp;
  char *ll;
  int max = file_size/1024 + ((file_size%1024==0)? 0:1);
  printf("max: %d\n", max);
  for (int i=0; i<max; i++)
  {
    n = recvfrom(sockfd, NULL, 0, MSG_PEEK | MSG_TRUNC, (struct sockaddr*)&servaddr, &len);
    printf("first pkt size %d\n",n);
    //pkt = malloc(n*sizeof(char));
    size +=n;
    tmp = buffer;
    buffer = malloc(size*sizeof(char));
    memcpy(buffer, tmp, size);
    recvfrom(sockfd, pkt, n, 0, (struct sockaddr*)(&servaddr),&len);
    b=size-n;

    cpy(pkt, buffer, b, n);
    temp = buffer;
    // send Packet_ACK_D
    pad.type = A;
    int ll = sizeof(servaddr);
    struct sockaddr* ad = (struct sockaddr*)&servaddr;
    sendto(sockfd,(const char*)&pad,1024,MSG_CONFIRM,ad, ll);
  }

  strcat(bname, ".out");
  FILE *f=fopen(bname,"wb");
  fwrite(temp, size, 1, f);
  fclose(f);
  return -1;
}

struct Packet_ACK_CC a_cc;

void close_connection_client()
{ printf("in\n");
  int n, len;
  n = recvfrom(sockfd, NULL, 0, MSG_PEEK|MSG_TRUNC, (struct sockaddr*)(&servaddr), &len);
  printf("***\n");

  recvfrom(sockfd, pkt, n, 0, (struct sockaddr*)(&servaddr),&len);
  printf("received closing connection %d\n",n);
  a_cc.type = C;
  sendto(sockfd, (const char*)&a_cc, 1024, MSG_PEEK|MSG_TRUNC, (const struct sockaddr*)&servaddr, sizeof(servaddr));
  if(close(sockfd) != 0)
    fprintf(stderr, "failed to close the client socket\n");
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
    } else {
      fname = argv[3];
    }
    
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      perror("socket creation failed");
      exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr.s_addr);

    handshake(servaddr);
    receive_data();
    close_connection_client();
  }
  return 0;
}
