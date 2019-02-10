#include "packets.h"

long PORT = 1080;
char *fname;
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

long file_size = 0;

struct Packet_SYN_ACK_C *desirealize_r(char *b)
{
  struct Packet_SYN_ACK_C *p = (struct Packet_SYN_ACK_C*)b;
  return p;
}

struct sockaddr_in servaddr, cliaddr;

struct Packet_ACK_C ack_c;
struct Packet_SYN_C syn;
char *bname;

int handshake(struct sockaddr_in servaddr)
{
  char bb[1024]; 
  syn.type = S;
  syn.pkt_len = sizeof(syn);
  bname = malloc(100);
  memset(bname, '\0', 100);
  strcpy(syn.filename, fname);
  printf("fname: %s\n", fname);
  memcpy(bname, basename(fname), sizeof(basename(fname)));
  printf("bname: %s\n", bname);
  int n, len;
sendto(sockfd,(const char*)serialize_s(&syn),1024,MSG_CONFIRM,(const struct sockaddr*)&servaddr,sizeof(servaddr));
  printf("---\n");

  n = recvfrom(sockfd, NULL, 0, MSG_PEEK | MSG_TRUNC, (struct sockaddr*)(&servaddr),&len);
  char* pkt = malloc(n * sizeof(char));
  printf("==%d===\n", n);
  //Read packet
  recvfrom(sockfd, pkt, n, 0, (struct sockaddr*)(&servaddr),&len);

  printf("received: %d\n",n);
  struct Packet_SYN_ACK_C *p = desirealize_r(pkt);
  printf("Server: %d %ld\n", p->type, p->file_size);
  file_size = p->file_size;
  ack_c.type = W;
  printf("send again\n");
sendto(sockfd,(const char*)serialize_w(&ack_c),1024,MSG_CONFIRM,(const struct sockaddr*)&servaddr,sizeof(servaddr));
  //  close(sockfd);
  return 1;
}

void cpy(void *src, void *dest, int i, int j)
{
  char *s = (char*)src;
  char *d = (char*)dest;
  for(int k=i; k<(i+j); k++)
    d[k] = s[k-i];
}

struct Packet_ACK_D pad;

int receive_data()
{
  int len, n;
  char *pkt;
  char *buffer;
  int size=0;
  char *temp;
  char *a;
  int b=0;
  char *tmp;
  int max = file_size/1024 + ((file_size%1024==0)? 0:1);
  printf("max: %d\n", max);
  for (int i=0; i<max; i++)
  {
    n = recvfrom(sockfd, NULL, 0, MSG_PEEK | MSG_TRUNC, (struct sockaddr*)&servaddr, &len);
    pkt = malloc(n*sizeof(char));
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
  }
  return 0;
}
