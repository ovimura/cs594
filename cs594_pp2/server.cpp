#include "server.hpp"

int DoCloseConnection(PacketStreamer &server_streamer, SEQUENCE_NUMBER_T &init_seq_num)
{
  Packet pkt;
  size_t recvd;

  printf("Closing Connection...\n");
  PacketStreamer::InitClosePacket(pkt, init_seq_num);
  server_streamer.SendPacket(pkt);
  recvd = server_streamer.RecvPacket(pkt);
  if (recvd && pkt.isType(PacketType::CLOSE_PACKET_TYPE) == true)
  {
    return 0;
  }
  else
  {
    printf("Invalid packet type\n");
    return EINVAL;
  }
  return 0;
}

int DoThreeWayHandShake(PacketStreamer &server_streamer, ifstream &input_stream)
{
  Packet pkt;
  size_t recvd;
  recvd = server_streamer.RecvPacket(pkt);
  if (recvd && pkt.isType(PacketType::SYN_PACKET_TYPE) == true)
  {
    string filename(pkt.GetOffsetPtr<char>(enum_cast(SYNPacketFields::FILE_NAME)));
    printf("Received SYN Packet\n");
    printf("Opening file: %s\n", filename.c_str());
    input_stream.open(filename, ifstream::in | ifstream::binary);
    if (input_stream.is_open())
    {
      PacketStreamer::InitSYNACKPacket(pkt, getFileSize(input_stream));
      printf("Sending SYN+ACK Packet\n");
      server_streamer.SendPacket(pkt);
      recvd = server_streamer.RecvPacket(pkt);
      if (pkt.isType(PacketType::CONNACK_PACKET_TYPE) == true)
      {
        printf("Received CONNACK Packet\n");
        return 0;
      }
      else
      {
        printf("Error in 3-way Handshake. Not a CONNACK PACKET\n");
        return EINVAL;
      }
    }
    else
    {
      printf("Error opening file\n");
      return EINVAL;
    }
  }
  else
  {
    printf("Error in 3-way Handshake. Not a SYN PACKET\n");
    return EINVAL;
  }

  return 0;
}

int main(int argc, char *argv[])
{
  UDPServerSocket server_socket;
  PacketStreamer server_streamer(&server_socket);
  ifstream input_stream;
  SEQUENCE_NUMBER_T init_seqnum = INIT_SEQNUM;
  GBNServer sw_server(server_streamer, MAX_PACKET_SIZE, ACK_TIMEOUT);
  int retval;
  int port;
  int w_size;

  if (argc != 3)
  {
    printf("Missing required parameter\n");
    printf("Usage: %s port window_size\n", argv[0]);
    return EINVAL;
  }

  port = strtol(argv[1], nullptr, 10);
  server_socket.Bind(port);
  printf("Waiting for connections on port: %u\n", port);
  w_size = strtol(argv[2], nullptr, 10);
  sw_server.SetWindowSize(w_size);
  retval = DoThreeWayHandShake(server_streamer, input_stream);
  if (retval == 0)
  {
    retval = sw_server.SendStream(&input_stream, init_seqnum);
    DoCloseConnection(server_streamer, init_seqnum);
  }

  server_socket.Close();
  input_stream.close();
  return 0;
}
