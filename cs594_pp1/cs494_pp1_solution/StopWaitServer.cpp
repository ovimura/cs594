#include "StopWaitServer.hpp"

StopWaitServer::StopWaitServer(PacketStreamer &streamer, PAYLOAD_LENGTH_T lmax_packet_size, int ack_timeout) : handler(this), shared_ctx(streamer)
{
  shared_ctx.timeout = ack_timeout;
  max_packet_size = lmax_packet_size;
  shared_ctx.acktimer.CreateTimer(&handler);
}

int StopWaitServer::SendStream(istream *input_stream, SEQUENCE_NUMBER_T &init_seq)
{
  int retval = 0;
  Packet ack_pkt;
  SEQUENCE_NUMBER_T pkt_seqnum;
  PAYLOAD_LENGTH_T bytes_read;
  bool retry = false;

  seqnum = init_seq;
  printf("Starting File transfer using Stop and Wait ARQ...\n");

  while (input_stream->good() || retry == true)
  {
    bytes_read = max_packet_size;
    shared_ctx.pktmutex.lock();
    if (retry == false)
    {
      PacketStreamer::InitDataPacket(shared_ctx.send_pkt, seqnum, input_stream, bytes_read);
      shared_ctx.server_streamer.SendPacket(shared_ctx.send_pkt);
    }

    printf("D[%04u](%u)->", seqnum, bytes_read);
    shared_ctx.pktmutex.unlock();

    fflush(stdout);
    shared_ctx.acktimer.SetTimer(shared_ctx.timeout);
    shared_ctx.server_streamer.RecvPacket(ack_pkt);
    shared_ctx.acktimer.CancelTimer();
    if (ack_pkt.isType(PacketType::DATAACK_PACKET_TYPE))
    {
      Socket::copy_ntoh(pkt_seqnum, *ack_pkt.GetOffsetPtr<const SEQUENCE_NUMBER_T>(enum_cast(DataACKPacketFields::SEQ_NUM)));
      shared_ctx.pktmutex.lock();
      if (pkt_seqnum == seqnum)
      {
        //ACCEPTED ACK
        printf("A[%04u]\n", seqnum);
        seqnum++;
        retry = false;
      }
      else
      {
        //DROPPED ACK
        printf("A[%04u] Dup:%04u\n*", pkt_seqnum, seqnum);
        retry = true;
      }
      shared_ctx.pktmutex.unlock();
	}
  }

  init_seq = seqnum;
  return retval;
}

void ack_handler::operator()()
{
  SEQUENCE_NUMBER_T pkt_seqnum;
  PAYLOAD_LENGTH_T bytes_read;
  int timeout;

  server_ptr->shared_ctx.pktmutex.lock();
  timeout = server_ptr->shared_ctx.timeout;
  Socket::copy_ntoh(pkt_seqnum, *server_ptr->shared_ctx.send_pkt.GetOffsetPtr<SEQUENCE_NUMBER_T>(enum_cast(DataPacketFields::SEQ_NUM)));
  Socket::copy_ntoh(bytes_read, *server_ptr->shared_ctx.send_pkt.GetOffsetPtr<PAYLOAD_LENGTH_T>(enum_cast(DataPacketFields::PAYLOAD_LENGHT)));
  server_ptr->shared_ctx.server_streamer.SendPacket(server_ptr->shared_ctx.send_pkt);
  server_ptr->shared_ctx.pktmutex.unlock();
  printf("X\n*D[%04u](%u)->", pkt_seqnum, bytes_read);
  server_ptr->shared_ctx.acktimer.SetTimer(timeout);
  server_ptr->shared_ctx.pktmutex.unlock();
}
