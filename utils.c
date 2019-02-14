#include "packets.h"

timer_t timer;
struct sigevent sigevt;
struct itimerspec timerspec;
int thread_param = 31415;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int expired=0;

void usage(char *a)
{
  printf("Usage: %s <server_ip> <port> <file>\n", a);
}

int isNumber(char* s)
{
  int len = strlen(s);
  for (int i=0; i<len; i++)
    if (!isdigit(s[i]))
    {
      printf("error: the entered <port> is not a number\n");
      exit(1);
    }
}

int validateFilePath(const char *f)
{
  FILE *file;
  if(file = fopen(f, "r"))
  {
    fclose(file);
    return 1;
  } else return 0;
}

int isServerIpFormat(char *ip)
{ char delim[] = ".";
  char *ptr = strtok(ip, delim);
  int i = 0;
  while(ptr != NULL)
  {
    ptr = strtok(NULL, delim);
    i++;
  }
  if(i==4)
    return 1;
  else
    return 0;
}

void cpy(void *src, void *dest, int i, int j)
{
  char *s = (char*)src;
  char *d = (char*)dest;
  for(int k=i; k<(i+j); k++)
    d[k] = s[k-i];
}


// ------------------- timer ------------------
void timer_thread(union sigval sv)
{
  pthread_mutex_lock(&mutex);
  expired = 1;
  pthread_mutex_unlock(&mutex); 
  printf("..timer_thread...\n");
}

void set_signal()
{
  sigevt.sigev_notify = SIGEV_THREAD;
  sigevt.sigev_notify_function = timer_thread;
  sigevt.sigev_value.sival_ptr = &thread_param;
  sigevt.sigev_notify_attributes = NULL;
}

void setup_one_shot_timer(int i)
{
  timerspec.it_value.tv_sec=0;
  timerspec.it_value.tv_nsec=1000000000;
  timerspec.it_interval.tv_sec=0;
  timerspec.it_interval.tv_nsec=0;
}

void arm_timer()
{
  timer_create(CLOCK_MONOTONIC, &sigevt, &timer);
  timer_settime(timer, 0, &timerspec, 0);
}

void disarm_timer()
{
  timerspec.it_value.tv_sec=0;
  timer_settime(timer, 0, &timerspec, 0);
//  setNotExpired();
}

int getExpired()
{
  return expired;
}

void setNotExpired()
{
  pthread_mutex_lock(&mutex);
  expired=0;
  pthread_mutex_unlock(&mutex);
  printf("info: expired set to 0\n");
}
// --------------------- timer ----------------------

int find_size(char file_name[])
{
  FILE* fp = fopen(file_name, "r");
  if(fp == NULL)
  {
    printf("File Not Found!\n");
    return -1;
  }
  fseek(fp, 0L, SEEK_END);
  int result = ftell(fp);
  fclose(fp);
  return result;
}

// ---------------------- desirealize ----------------------
struct Packet_SYN_C *desirealize_s(char *b)
{
  struct Packet_SYN_C *p = malloc(sizeof(struct Packet_SYN_C));
  memcpy(&((struct Packet_SYN_C*)p)->type, &((struct Packet_SYN_C*)b)->type, sizeof(char));
  memcpy(&((struct Packet_SYN_C*)p)->pkt_len, &((struct Packet_SYN_C*)b)->pkt_len, sizeof(short));
  memcpy(&((struct Packet_SYN_C *)p)->filename,&((struct Packet_SYN_C *)b)->filename, sizeof(char)*100);
  return p;
}

struct Packet_ACK_C *desirealize_w(char *b)
{
  struct Packet_ACK_C *p = malloc(sizeof(char));
  memcpy(&((struct Packet_ACK_C*)p)->type, b, sizeof(char));
  printf("type: packet_ack_c: %d\n",p->type);
  return p;
}


struct Packet_SYN_ACK_C *desirealize_r(char *b)
{
  struct Packet_SYN_ACK_C *p = malloc(sizeof(struct Packet_SYN_ACK_C));
  memcpy(&((struct Packet_SYN_ACK_C*)p)->type, &((struct Packet_SYN_ACK_C*)b)->type, sizeof(char));
  memcpy(&((struct Packet_SYN_ACK_C*)p)->file_size, b+1, sizeof(long));
  return p;
}

char * serialize_r(struct Packet_SYN_ACK_C *ppp)
{
  long sii = sizeof(char)+sizeof(long)+4;
  char *r = malloc(sii);
  int off = 0;
  memcpy(r, &ppp->type, sizeof(char));
  off=sizeof(char);
  memcpy(r+off, &ppp->file_size, sizeof(long)+2);
  return r;
}


char * serialize_s(struct Packet_SYN_C *p)
{
  char *s = malloc(sizeof(char)+sizeof(short)+sizeof(char)*100);
  int off=0;
  memcpy(s, &p->type, sizeof(char));
  off = sizeof(char);
  memcpy(s+off+1, &p->pkt_len, sizeof(short));
  off += sizeof(short);
  memcpy(s+off+1, &p->filename, sizeof(char)*98);
  return s;
}

char * serialize_w(struct Packet_ACK_C *p)
{
  char *m = malloc(sizeof(char));
  memcpy(m, &p->type, sizeof(char));
  return m;
}

char *serialize_d(struct Packet_DATA_D *p)
{
  char *d = malloc(sizeof(char)+sizeof(int)+sizeof(short)+sizeof(char)*1024);
  int off=0;
  memcpy(d, &p->type, sizeof(char));
  off = sizeof(char);
  memcpy(d+off, &p->seq_num, sizeof(int));
  off += sizeof(int);
  memcpy(d+off, &p->pkt_len, sizeof(short));
  off += sizeof(short);
  memcpy(d+off, &p->data, sizeof(char)*1024);
  return d;
}

struct Packet_DATA_D *desirealize_d(char *d)
{
  struct Packet_DATA_D *p = malloc(sizeof(struct Packet_DATA_D));
  int off=0;
  memcpy(&p->type, d, sizeof(char));
  off+=sizeof(char);
  memcpy(&p->seq_num, d+off, sizeof(int));
  off+=sizeof(int);
  memcpy(&p->pkt_len, d+off, sizeof(short));
  off+=sizeof(short);
  memcpy(&p->data, d+off, 1024);
  return p;
}
