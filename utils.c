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
