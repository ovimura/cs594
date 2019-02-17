#include<stdio.h>
#include<time.h>
#include<signal.h>
#include<unistd.h>


void timer_thread(union sigval sv)
{
  printf("Hello from timer. Argument: %d\n", *(int*)sv.sival_ptr);
}


int main()
{
  timer_t timer;
  struct sigevent sigevt;
  struct itimerspec timerspec;
  int thread_param = 31415;

  sigevt.sigev_notify = SIGEV_THREAD;
  sigevt.sigev_notify_function = timer_thread;
  sigevt.sigev_value.sival_ptr = &thread_param;
  sigevt.sigev_notify_attributes = NULL;
//  timer_create(CLOCK_MONOTONIC, &sigevt, &timer);

  //SETUP ONE-SHOT TIMER
  timerspec.it_value.tv_sec=5; //Expire after 1 second
  timerspec.it_value.tv_nsec=0;
  timerspec.it_interval.tv_sec=0; //One shot timer
  timerspec.it_interval.tv_nsec=0;

  for (int i=0; i<1; i++)
  {
    printf("enter time\n");
    timer_create(CLOCK_MONOTONIC, &sigevt, &timer);
    timer_settime(timer, 0, &timerspec, 0);
    timerspec.it_value.tv_sec=0;
    timer_settime(timer, 0, &timerspec, 0);
    printf("the time executed once\n");
  }

  sleep(30);

  printf("--------\n");
  return 0;
}
