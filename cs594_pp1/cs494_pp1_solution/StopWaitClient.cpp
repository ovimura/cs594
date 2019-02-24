#include "StopWaitClient.hpp"

int StopWaitClient::ProcessDataPacket(ostream *output_stream, const Packet &recv_pkt, SEQUENCE_NUMBER_T &pkt_seqnum, PAYLOAD_LENGTH_T &payload_len)
{
  Socket::copy_ntoh(pkt_seqnum, *recv_pkt.GetOffsetPtr<const SEQUENCE_NUMBER_T>(enum_cast(DataPacketFields::SEQ_NUM)));
  if (pkt_seqnum == seqnum)
  {
    Socket::copy_ntoh(payload_len, *recv_pkt.GetOffsetPtr<const PAYLOAD_LENGTH_T>(enum_cast(DataPacketFields::PAYLOAD_LENGHT)));
    const char *ptr = recv_pkt.GetOffsetPtr<const char>(enum_cast(DataPacketFields::DATA));
    output_stream->write(ptr, payload_len);
    return 0;
  }
  else
  {
    payload_len = 0;
    return EINVAL;
  }
}

int StopWaitClient::RecvStream(ostream *output_stream, FILE_SIZE_T filesize, SEQUENCE_NUMBER_T &init_seqnum)
{
  Packet ack_pkt;
  Packet recv_pkt;
  int retval = 0;
  PAYLOAD_LENGTH_T bytes_read = 0;
  FILE_SIZE_T total_received_bytes = 0;
  SEQUENCE_NUMBER_T pkt_seqnum = 0;

  seqnum = init_seqnum;

  printf("Starting File transfer using Stop and Wait ARQ...\n");
  while (total_received_bytes < filesize)
  {
    client_streamer.RecvPacket(recv_pkt);
    if (recv_pkt.isType(PacketType::DATA_PACKET_TYPE))
    {
      retval = ProcessDataPacket(output_stream, recv_pkt, pkt_seqnum, bytes_read);
      if (retval != 0)
      {
        //DROPPED PACKET: DUPLICATE PACKET!!!
        printf("D[%04u](%u) Dup:%04u <-*", pkt_seqnum, bytes_read, seqnum);
      }
      else
      {
        //ACCEPTED PACKET
        printf("D[%04u](%u)<-", pkt_seqnum, bytes_read);
        total_received_bytes += bytes_read;
        seqnum++;
      }
      // REPLY WITH ACK[pkt_seqnum] even if duplicate
      PacketStreamer::InitDataACKPacket(ack_pkt, pkt_seqnum);
      client_streamer.SendPacket(ack_pkt);
      printf("A[%04u]\n", pkt_seqnum);
    }
  }

  printf("File received:%lu bytes\n", total_received_bytes);
  init_seqnum = seqnum;
  return retval;
}
