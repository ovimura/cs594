#pragma once
#include <istream>
#include <mutex>
#include "udpsocket.hpp"
#include "Packets.hpp"
#include "timer.hpp"
#include <queue>


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

  SharedContext(const SharedContext &) = delete;
  SharedContext &operator=(const SharedContext &) = delete;
};

class Reciver
{
public:
  Reciver();
  void start();
  void join(pthread_t);
  void * handler_receiver(void*);

private:
  pthread_t threadid = -1;
  pthread_attr_t *attr = NULL;
  void *(*start_routine)(void*) = NULL;
  void *arg = NULL;
};

class GBNServer
{
public:
  GBNServer(PacketStreamer &streamer, PAYLOAD_LENGTH_T lmax_packet_size, int ack_timeout);
  int SendStream(istream *input_stream, SEQUENCE_NUMBER_T &init_seq);
  void SetWindowSize(int w_size);

private:
  PAYLOAD_LENGTH_T max_packet_size;
  SEQUENCE_NUMBER_T seqnum;
  ack_handler handler;
  class SharedContext shared_ctx;
  class Reciver reciver;
  friend class ack_handler;

  GBNServer(const GBNServer &) = delete;
  GBNServer &operator=(const GBNServer &) = delete;
};
