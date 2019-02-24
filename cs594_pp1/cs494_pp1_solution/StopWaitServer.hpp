#pragma once
#include <istream>
#include <mutex>
#include "udpsocket.hpp"
#include "Packets.hpp"
#include "timer.hpp"

using namespace std;

class StopWaitServer;

class ack_handler : public timer_handler
{
public:
  ack_handler(StopWaitServer *ctx) : server_ptr(ctx) {}
  void operator()();

private:
  StopWaitServer *server_ptr;
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

  SharedContext(const SharedContext &) = delete;
  SharedContext &operator=(const SharedContext &) = delete;
};

class StopWaitServer
{
public:
  StopWaitServer(PacketStreamer &streamer, PAYLOAD_LENGTH_T lmax_packet_size, int ack_timeout);
  int SendStream(istream *input_stream, SEQUENCE_NUMBER_T &init_seq);

private:
  PAYLOAD_LENGTH_T max_packet_size;
  SEQUENCE_NUMBER_T seqnum;
  ack_handler handler;
  class SharedContext shared_ctx;
  friend class ack_handler;

  StopWaitServer(const StopWaitServer &) = delete;
  StopWaitServer &operator=(const StopWaitServer &) = delete;
};
