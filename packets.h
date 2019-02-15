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
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <libgen.h>


enum packet_type {S, R, W, D, A, C};

struct Packet_SYN_C {
  char type;
  short pkt_len;
  char filename[100];
};

struct Packet_SYN_ACK_C {
  char type;
  long file_size;
};

struct Packet_ACK_C {
  char type;
};

struct Packet_DATA_D {
  char type;
  int seq_num;
  short pkt_len;
  char data[1024];
};

struct Packet_ACK_D {
  char type;
  int seq_num;
};

struct Packet_ACK_CC {
  char type;
  int seq_num;
};


int isNumber(char *);
int validateFilePath(const char *);
void usage(char *);
int isServerIpFormat(char *);
void timer_thread(union sigval);
void set_signal();
void setup_one_shot_timer(int);
void arm_timer();
void disarm_timer();
int getExpired();
void setNotExpired();
int find_size(char file_name[]);
void cpy(void *src, void *dest, int i, int j);

char *serialize_r(struct Packet_SYN_ACK_C *r);
char *serialize_s(struct Packet_SYN_C *s);
char *serialize_w(struct Packet_ACK_C *w);
char *serialize_d(struct Packet_DATA_D *d);
char *serialize_ad(struct Packet_ACK_D *ad);
char *serialize_cc(struct Packet_ACK_CC *cc);

struct Packet_ACK_CC *desirealize_cc(char *cc);
struct Packet_ACK_D *desirealize_ad(char *ad);
struct Packet_DATA_D *desirealize_d(char *d);
struct Packet_SYN_C *desirealize_s(char *s);
struct Packet_ACK_C *desirealize_w(char *w);
struct Packet_SYN_ACK_C *desirealize_r(char *r);

