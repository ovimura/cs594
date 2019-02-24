#include "client.hpp"

int DoCloseConnection(PacketStreamer &client_streamer, SEQUENCE_NUMBER_T &init_seq_num)
{
  Packet pkt;
  size_t recvd;

  printf("Waiting for CLosing Packet...\n");

  recvd = client_streamer.RecvPacket(pkt);
  while (recvd && pkt.isType(PacketType::CLOSE_PACKET_TYPE) != true)
  {
    printf("Invalid packet type\n");
    recvd = client_streamer.RecvPacket(pkt);
  }

  PacketStreamer::InitClosePacket(pkt, init_seq_num);
  client_streamer.SendPacket(pkt);
  return 0;
}

int DoThreeWayHandShake(PacketStreamer &client_streamer, const string &filename, size_t &filesize)
{
  Packet pkt;
  size_t recvd;

  PacketStreamer::InitSYNPacket(pkt, filename);
  printf("Sending SYN Packet\n");
  client_streamer.SendPacket(pkt);

  recvd = client_streamer.RecvPacket(pkt);
  if (recvd && pkt.isType(PacketType::SYNACK_PACKET_TYPE) == true)
  {
    Socket::copy_ntoh(filesize, *pkt.GetOffsetPtr<FILE_SIZE_T>(enum_cast(SYNACKPacketFields::FILE_SIZE)));
    printf("Received SYN+ACK PACKET. Filesize:%lu\n", filesize);
    PacketStreamer::InitConnACKPacket(pkt);
    printf("Sending CONNACK Packet\n");
    client_streamer.SendPacket(pkt);
    return 0;
  }
  else
  {
    printf("Error in 3-way Handshake. Not a SYN+ACK PACKET\n");
    return EINVAL;
  }
}

int main(int argc, char *argv[])
{
  SEQUENCE_NUMBER_T seq_num = INIT_SEQNUM;
  UDPClientSocket client_socket;
  PacketStreamer client_streamer(&client_socket);
  ofstream output_stream;
  StopWaitClient sw_client(client_streamer);
  size_t filesize = 0;
  int port;
  int retval;

  if (argc != 4)
  {
    printf("Missing required parameter\n");
    printf("Usage: %s port server_ip filename\n", argv[0]);
    return EINVAL;
  }

  string filename(argv[3]);
  port = strtol(argv[1], nullptr, 10);
  client_socket.Bind(argv[2], port);
  printf("Contacting server: %s on port: %u. File: %s\n", argv[2], port, argv[3]);
  retval = DoThreeWayHandShake(client_streamer, filename, filesize);
  if (retval == 0)
  {
    filename.append(".out");
    output_stream.open(filename, ofstream::out | ofstream::trunc | ofstream::binary);
    if (!output_stream.is_open())
    {
      printf("Error opening output file: %s\n", filename.c_str());
      return EINVAL;
    }
//    retval = sw_client.RecvStream(&output_stream, filesize, seq_num);
    DoCloseConnection(client_streamer, seq_num);
  }

  client_socket.Close();
  output_stream.close();
  return 0;
}
