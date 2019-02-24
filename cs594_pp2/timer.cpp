#include "timer.hpp"

void sigev_notify_wrapper(union sigval param)
{
  Timer *timer_ptr = reinterpret_cast<Timer *>(param.sival_ptr);
  (*timer_ptr->timer_thread)();
}

int Timer::CancelTimer()
{
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 0;
  return SetTimer(ts);
}

int Timer::SetTimer(const int timeout_ms)
{
  struct timespec ts;
  ts.tv_sec = timeout_ms / 1000;
  ts.tv_nsec = (timeout_ms % 1000) * 1000000;
  return SetTimer(ts);
}

int Timer::SetTimer(const struct timespec &timeout)
{
  int retval;
  struct itimerspec ts;

  if (timer_thread == nullptr)
  {
    assert(timer_thread);
    printf("Timer not Initialized\n");
    return EINVAL;
  }

  trivial_zero(&ts, 1);
  ts.it_value = timeout;
  retval = timer_settime(htimer, 0, &ts, nullptr);
  if (retval == -1)
  {
    retval = errno;
    printf("Error Setting Timer: %d\n", retval);
    return retval;
  }
  return 0;
}

int Timer::CreateTimer(timer_handler *handler)
{
  struct sigevent sigevt;
  int retval;

  trivial_zero(&sigevt, 1);

  sigevt.sigev_notify = SIGEV_THREAD;
  sigevt.sigev_notify_function = sigev_notify_wrapper;
  sigevt.sigev_value.sival_ptr = this;
  sigevt.sigev_notify_attributes = nullptr;

  retval = timer_create(CLOCK_MONOTONIC, &sigevt, &htimer);
  if (retval == -1)
  {
    retval = errno;
    printf("Error Creating Timer: %d\n", retval);
    return retval;
  }

  timer_thread = handler;
  return 0;
}
