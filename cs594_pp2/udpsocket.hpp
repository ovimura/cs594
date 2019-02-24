#pragma once
#include <string>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <errno.h>
#include "utility.hpp"

using namespace std;

class Socket
{
public:
  size_t Send(const void *buffer, size_t size);
  size_t Recv(void *buffer, size_t size);
  size_t Peek(void *buffer, size_t size);
  virtual ~Socket();
  void Close();
  static void copy_hton(uint16_t& dest,const uint16_t src);
  static void copy_hton(uint32_t& dest,const uint32_t src);
  static void copy_hton(uint64_t& dest,const uint64_t src);
  static void copy_ntoh(uint16_t& dest,const uint16_t src);
  static void copy_ntoh(uint32_t& dest,const uint32_t src);
  static void copy_ntoh(uint64_t& dest,const uint64_t src);

protected:
  Socket();

  int sockid;
  struct sockaddr_in addr;
};

class UDPClientSocket : public Socket
{
public:
  UDPClientSocket() : Socket() {}
  int Bind(string server_ip, in_port_t server_port);
};

class UDPServerSocket : public Socket
{
public:
  UDPServerSocket() : Socket() {}
  int Bind(in_port_t server_port);
};
