#pragma once
#include <istream>
#include <mutex>
#include "udpsocket.hpp"
#include "Packets.hpp"
#include "timer.hpp"
#include <queue>
#include <pthread.h>
#include <cmath>

using namespace std;

class GBNServer;

class ack_handler : public timer_handler
{
public:
  ack_handler(GBNServer *ctx) : server_ptr(ctx) {}
  void operator()();

private:
  GBNServer *server_ptr;
};

enum class State : uint8_t
{
  AVAILABLE = 'A',
  SENT = 'S',
  ACKNOWLEDGED = 'K',
  EXPIRED = 'E'
}; 

class SPacket
{
public:
  SPacket(Packet &p, State s);
  State state;
  Packet pkt;
};


class SharedContext
{
public:
  SharedContext(PacketStreamer &streamer) : server_streamer(streamer) {}
  mutex pktmutex;
  Packet send_pkt;
  int timeout;
  PacketStreamer server_streamer;
  Timer acktimer;
  queue<SPacket> q;
  int q_size;
  size_t last_window;
  SEQUENCE_NUMBER_T seqnum = 0;
  pthread_cond_t cond;
  pthread_mutex_t win_mutex; // = PTHREAD_MUTEX_INITIALIZER;
  void SetStateLPkt(State);
  SEQUENCE_NUMBER_T sn;

  SharedContext(const SharedContext &) = delete;
  SharedContext &operator=(const SharedContext &) = delete;
};

class Receiver
{
public:
  Receiver(){}
  void start(SharedContext *);
  void join(pthread_t);
  friend void * handler_receiver(void*);
  class SharedContext *s_context;

private:
  pthread_t threadid = -1;
  pthread_attr_t *attr = NULL;
  void *(*start_routine)(void*) = NULL;
  void *arg = NULL;
};

class GBNServer
{
public:
  GBNServer(PacketStreamer &streamer, PAYLOAD_LENGTH_T lmax_packet_size, int ack_timeout, int w_size);
  int SendStream(istream *input_stream, SEQUENCE_NUMBER_T &init_seq);
  void SetWindowSize(int w_size);
  int Push(SPacket);

private:
  PAYLOAD_LENGTH_T max_packet_size;
  SEQUENCE_NUMBER_T seqnum;
  ack_handler handler;
  class SharedContext shared_ctx;
  class Receiver receiver;
  friend class ack_handler;

  GBNServer(const GBNServer &) = delete;
  GBNServer &operator=(const GBNServer &) = delete;
};
