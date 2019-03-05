#pragma once

#include <signal.h>
#include <time.h>
#include <cstring>
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include "utility.hpp"

class timer_handler
{
public:
  virtual void operator()() = 0;
  virtual ~timer_handler() {}
};

class Timer
{
public:
  Timer() : timer_thread(nullptr) {}
  int CreateTimer(timer_handler *handler);
  int SetTimer(const struct timespec &timeout);
  int SetTimer(const int timeout_ms);
  int CancelTimer();
  int IsSet = 0;

private:
  friend void sigev_notify_wrapper(union sigval param);
  timer_t htimer;
  timer_handler *timer_thread;
};
