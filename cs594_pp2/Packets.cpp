#include "Packets.hpp"

size_t PacketStreamer::SendPacket(const Packet &pkt)
{
  if (socket != nullptr)
    return socket->Send(pkt.data(), pkt.size());
  else
    return 0;
}

size_t PacketStreamer::RecvPacket(Packet &pkt)
{
  size_t pkt_size;

  pkt.clear();
  if (socket != nullptr)
  {
    pkt_size = socket->Peek(nullptr, 0);
    pkt.resize(pkt_size);
    socket->Recv(pkt.data(), pkt_size);
    return pkt_size;
  }
  else
    return 0;
}

void PacketStreamer::InitDataACKPacket(Packet &pkt, SEQUENCE_NUMBER_T seqnum)
{
  pkt.resize(DATAACKPacket_SIZE);
  trivial_zero<uint8_t>(pkt.data(), DATAACKPacket_SIZE);

  *pkt.GetOffsetPtr<PACKET_TYPE_T>(enum_cast(DataACKPacketFields::PACKET_TYPE))= enum_cast(PacketType::DATAACK_PACKET_TYPE);
  Socket::copy_hton(*pkt.GetOffsetPtr<SEQUENCE_NUMBER_T>(enum_cast(DataACKPacketFields::SEQ_NUM)), seqnum);
}

void PacketStreamer::InitClosePacket(Packet &pkt, SEQUENCE_NUMBER_T seqnum)
{
  pkt.resize(CLOSEPacket_SIZE);
  trivial_zero<uint8_t>(pkt.data(), CLOSEPacket_SIZE);

  *pkt.GetOffsetPtr<PACKET_TYPE_T>(enum_cast(ClosePacketFields::PACKET_TYPE)) = enum_cast(PacketType::CLOSE_PACKET_TYPE);
}

void PacketStreamer::InitConnACKPacket(Packet &pkt)
{
  pkt.resize(CONNACKPacket_SIZE);
  trivial_zero<uint8_t>(pkt.data(), CONNACKPacket_SIZE);
  *pkt.GetOffsetPtr<PACKET_TYPE_T>(enum_cast(ConnACKPacketFields::PACKET_TYPE)) = enum_cast(PacketType::CONNACK_PACKET_TYPE);
}

void PacketStreamer::InitDataPacket(Packet &pkt, SEQUENCE_NUMBER_T seqnum, istream *stream, PAYLOAD_LENGTH_T &payload_len)
{
  pkt.resize(DATAPacket_SIZE + payload_len);
  trivial_zero<uint8_t>(pkt.data(), SYNPacket_SIZE + payload_len);

  *pkt.GetOffsetPtr<PACKET_TYPE_T>(enum_cast(DataPacketFields::PACKET_TYPE)) = enum_cast(PacketType::DATA_PACKET_TYPE);

  Socket::copy_hton(*pkt.GetOffsetPtr<SEQUENCE_NUMBER_T>(enum_cast(DataPacketFields::SEQ_NUM)), seqnum);

  char *ptr = pkt.GetOffsetPtr<char>(enum_cast(DataPacketFields::DATA));
  stream->read(ptr, payload_len);
  PAYLOAD_LENGTH_T bytes_read = stream->gcount();

  payload_len = min<PAYLOAD_LENGTH_T>(payload_len, bytes_read);
  Socket::copy_hton(*pkt.GetOffsetPtr<PAYLOAD_LENGTH_T>(enum_cast(DataPacketFields::PAYLOAD_LENGHT)), payload_len);
}

void PacketStreamer::InitSYNACKPacket(Packet &pkt, FILE_SIZE_T filesize)
{
  pkt.resize(SYNACKPacket_SIZE);
  trivial_zero<uint8_t>(pkt.data(), SYNACKPacket_SIZE);

  *pkt.GetOffsetPtr<PACKET_TYPE_T>(enum_cast(SYNACKPacketFields::PACKET_TYPE)) = enum_cast(PacketType::SYNACK_PACKET_TYPE);

  Socket::copy_hton(*pkt.GetOffsetPtr<FILE_SIZE_T>(enum_cast(SYNACKPacketFields::FILE_SIZE)), filesize);
}

void PacketStreamer::InitSYNPacket(Packet &pkt, const string &filename)
{
  size_t filename_len = filename.size() + 1;
  pkt.resize(SYNPacket_SIZE + filename_len);
  trivial_zero<uint8_t>(pkt.data(), SYNPacket_SIZE + filename_len);

  *pkt.GetOffsetPtr<PACKET_TYPE_T>(enum_cast(SYNPacketFields::PACKET_TYPE)) = enum_cast(PacketType::SYN_PACKET_TYPE);

  Socket::copy_hton(*pkt.GetOffsetPtr<PAYLOAD_LENGTH_T>(enum_cast(SYNPacketFields::PAYLOAD_LENGTH)),filename_len);

  char *ptr = pkt.GetOffsetPtr<char>(enum_cast(SYNPacketFields::FILE_NAME));
  strcpy(ptr, filename.c_str());
}
