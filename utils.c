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
  timerspec.it_value.tv_sec=i;
  timerspec.it_value.tv_nsec=0;
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

struct Packet_ACK_C *pp;
struct Packet_ACK_C *desirealize_w(char *b)
{
  pp = (struct Packet_ACK_C*)b;
  return pp;
}


struct Packet_SYN_ACK_C *desirealize_r(char *b)
{
  struct Packet_SYN_ACK_C *p = (struct Packet_SYN_ACK_C*)b;
  return p;
}

char r[1024];
char * serialize_r(struct Packet_SYN_ACK_C *ppp)
{
  memcpy(r, (const char*)ppp, 1024);
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

char m[1024];

char * serialize_w(struct Packet_ACK_C *p)
{
  memcpy(m, (const char*)p, 1024);
  return m;
}

