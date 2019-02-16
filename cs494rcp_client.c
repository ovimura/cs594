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
  free(ser);
  n = recvfrom(sockfd, NULL, 0, MSG_PEEK | MSG_TRUNC, (struct sockaddr*)(&servaddr),&len);
  //Read packet
  recvfrom(sockfd, pkt, n, 0, (struct sockaddr*)(&servaddr),&len);

  struct Packet_SYN_ACK_C *p = desirealize_r(pkt);
  file_size = p->file_size;
  free(p);

  ack_c.type = W;
  char *ser_w = serialize_w(&ack_c);
  sendto(sockfd,ser_w,sizeof(ack_c),MSG_PEEK|MSG_TRUNC,(const struct sockaddr*)&servaddr,sizeof(servaddr));
  free(ser_w);
  return 1;
}

struct Packet_ACK_D pad;
char *pkt_ack_d;
char pktt[2048];
int receive_data()
{
  int len, n;
  char *buffer=NULL;
  int size=0;
  char *temp;
  char *a;
  int b=0;
  char *tmp;
  char *ll;
  int max = file_size/1024 + ((file_size%1024==0)? 0:1);
  for (int i=0; i<max; i++)
  {
    n = recvfrom(sockfd, NULL, 0, MSG_PEEK | MSG_TRUNC, (struct sockaddr*)&servaddr, &len);
    tmp = buffer;
    recvfrom(sockfd, pktt, n, 0, (struct sockaddr*)(&servaddr),&len);

    struct Packet_DATA_D *dd = desirealize_d(pktt);
    if(dd->seq_num != i)
      i--;
    if(dd->seq_num == i)
    {
      size += dd->pkt_len;
      buffer = malloc(size*sizeof(char));

      if(tmp!=NULL)
        memcpy(buffer, tmp, size);

      b=size-dd->pkt_len;
      cpy(&dd->data, buffer, b, dd->pkt_len);
      temp = buffer;
      free(dd);
    }
    pad.type = A;
    pad.seq_num = i;
    pkt_ack_d = serialize_ad(&pad);

    // send acknowledge
    int ll = sizeof(servaddr);
    struct sockaddr* ad = (struct sockaddr*)&servaddr;
    long ss = sizeof(pad);
    sendto(sockfd,pkt_ack_d,ss,MSG_CONFIRM,ad, ll);
    free(pkt_ack_d);
  }

  strcat(bname, ".out");
  FILE *f=fopen(bname,"wb");
  fwrite(temp, size, 1, f);
  fclose(f);
  return -1;
}

struct Packet_ACK_CC a_cc;

void close_connection_client()
{ 
  int n, len;
  n = recvfrom(sockfd, NULL, 0, MSG_PEEK|MSG_TRUNC, (struct sockaddr*)(&servaddr), &len);

  recvfrom(sockfd, pkt, n, 0, (struct sockaddr*)(&servaddr),&len);
  struct Packet_ACK_CC *acc = desirealize_cc(pkt);
  a_cc.type = C;
  char * packet_ack_cc = serialize_cc(&a_cc);
  sendto(sockfd, packet_ack_cc, 100, MSG_PEEK|MSG_TRUNC, (const struct sockaddr*)&servaddr, sizeof(servaddr));
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
