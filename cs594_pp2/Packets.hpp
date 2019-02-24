#pragma once
#include <cstdint>
#include <vector>
#include <cassert>
#include "utility.hpp"
#include "udpsocket.hpp"

using namespace std;

enum class PacketType : uint8_t
{
  SYN_PACKET_TYPE = 'S',
  SYNACK_PACKET_TYPE = 'R',
  CONNACK_PACKET_TYPE = 'W',
  DATA_PACKET_TYPE = 'D',
  DATAACK_PACKET_TYPE = 'A',
  CLOSE_PACKET_TYPE = 'C'
};

#define SYNPacket_SIZE (sizeof(PACKET_TYPE_T) + sizeof(PAYLOAD_LENGTH_T))
#define SYNACKPacket_SIZE (enum_cast(SYNACKPacketFields::FILE_SIZE) + sizeof(FILE_SIZE_T))
#define CONNACKPacket_SIZE sizeof(PACKET_TYPE_T)
#define DATAPacket_SIZE enum_cast(DataPacketFields::DATA)
#define DATAACKPacket_SIZE (enum_cast(DataACKPacketFields::SEQ_NUM) + sizeof(SEQUENCE_NUMBER_T))
#define CLOSEPacket_SIZE (enum_cast(ClosePacketFields::SEQ_NUM) + sizeof(SEQUENCE_NUMBER_T))

typedef uint8_t PACKET_TYPE_T;
typedef uint16_t PAYLOAD_LENGTH_T;
typedef uint32_t SEQUENCE_NUMBER_T;
typedef uint64_t FILE_SIZE_T;

/* This works because sizeof() is of type const_expr */
enum class SYNPacketFields : size_t
{
  PACKET_TYPE = 0,
  PAYLOAD_LENGTH = sizeof(PACKET_TYPE_T),
  FILE_NAME = sizeof(PACKET_TYPE_T) + sizeof(PAYLOAD_LENGTH_T)
};

enum class SYNACKPacketFields : size_t
{
  PACKET_TYPE = 0,
  FILE_SIZE = sizeof(PACKET_TYPE_T)
};

enum class ConnACKPacketFields : size_t
{
  PACKET_TYPE = 0
};

enum class DataPacketFields : size_t
{
  PACKET_TYPE = 0,
  SEQ_NUM = sizeof(PACKET_TYPE_T),
  PAYLOAD_LENGHT = sizeof(PACKET_TYPE_T) + sizeof(SEQUENCE_NUMBER_T),
  DATA = sizeof(PACKET_TYPE_T) + sizeof(SEQUENCE_NUMBER_T) + sizeof(PAYLOAD_LENGTH_T)
};

enum class DataACKPacketFields : size_t
{
  PACKET_TYPE = 0,
  SEQ_NUM = sizeof(PACKET_TYPE_T)
};

enum class ClosePacketFields : size_t
{
  PACKET_TYPE = 0,
  SEQ_NUM = sizeof(PACKET_TYPE_T)
};

class Packet : public vector<uint8_t>
{
public:
  bool isType(PacketType type) const { return *GetPacketTypePtr() == enum_cast(type); };
  const PACKET_TYPE_T *GetPacketTypePtr() const { return data(); }
  PACKET_TYPE_T *GetPacketTypePtr() { return data(); }
  template <class T>
  T *GetOffsetPtr(size_t offset);
  template <class T>
  const T *GetOffsetPtr(size_t offset) const;
};

template <class T>
T *Packet::GetOffsetPtr(size_t offset)
{
  if (offset < size())
    return reinterpret_cast<T *>(data() + offset);
  else
  {
    assert(0);
    return nullptr;
  }
}

template <class T>
const T *Packet::GetOffsetPtr(size_t offset) const
{
  if (offset < size())
    return reinterpret_cast<T *>(data() + offset);
  else
  {
    assert(0);
    return nullptr;
  }
}

class PacketStreamer
{
public:
  PacketStreamer(Socket *lsocket) { socket = lsocket; }
  size_t SendPacket(const Packet &pkt);
  size_t RecvPacket(Packet &pkt);

  static void InitSYNPacket(Packet &pkt, const string &filename);
  static void InitSYNACKPacket(Packet &pkt, FILE_SIZE_T filesize);
  static void InitConnACKPacket(Packet &pkt);
  static void InitDataPacket(Packet &pkt, SEQUENCE_NUMBER_T seqnum, istream *stream, PAYLOAD_LENGTH_T &payload_len);
  static void InitDataACKPacket(Packet &pkt, SEQUENCE_NUMBER_T seqnum);
  static void InitClosePacket(Packet &pkt, SEQUENCE_NUMBER_T seqnum);

private:
  Socket *socket;
};
