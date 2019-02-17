#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

//long PORT = 8080;

enum packet_type {S, R, W, D, A, C};

struct Packet_SYN_C {
  char type;
  short pkt_len;
  char filename[100];
};

struct Packet_SYN_ACK_C {
  enum packet_type type;
  long file_size;
};

struct Packet_ACK_C {
  enum packet_type type;
};

struct Packet_DATA_D {
  enum packet_type type;
  int seq_num;
  short pkt_len;
  char data[1024];
};

struct Packet_ACK_D {
  enum packet_type type;
  int seq_num;
};

struct Packet_ACK_CC {
  enum packet_type type;
  int seq_num;
};

int isNumber(char *);
int validateFilePath(const char *);
void usage(char *);
int isServerIpFormat(char *);
