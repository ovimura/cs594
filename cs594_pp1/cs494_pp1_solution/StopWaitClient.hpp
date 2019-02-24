#pragma once
#include <ostream>
#include "udpsocket.hpp"
#include "Packets.hpp"

using namespace std;

class StopWaitClient
{
public:
  StopWaitClient(const PacketStreamer &streamer) : client_streamer(streamer) {}
  int RecvStream(ostream *output_stream, FILE_SIZE_T filesize, SEQUENCE_NUMBER_T &pkt_seqnum);

private:
  PacketStreamer client_streamer;
  SEQUENCE_NUMBER_T seqnum;
  int ProcessDataPacket(ostream *output_stream, const Packet &recv_pkt, SEQUENCE_NUMBER_T &pkt_seqnum, PAYLOAD_LENGTH_T &payload_len);
  StopWaitClient(const StopWaitClient &) = delete;
  StopWaitClient &operator=(const StopWaitClient &) = delete;
};
