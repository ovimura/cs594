#include "udpsocket.hpp"

Socket::~Socket()
{
  Close();
}

Socket::Socket()
{
  sockid = -1;
  trivial_zero(&addr, 1);
}

void Socket::Close()
{
  if (sockid != -1)
    close(sockid);
  sockid = -1;
}

size_t Socket::Send(const void *buffer, size_t size)
{
  return sendto(sockid, buffer, size, 0, reinterpret_cast<const struct sockaddr *>(&addr), sizeof(struct sockaddr_in));
}

size_t Socket::Recv(void *buffer, size_t size)
{
  socklen_t sizesockaddr = sizeof(struct sockaddr_in);
  return recvfrom(sockid, buffer, size, 0, reinterpret_cast<struct sockaddr *>(&addr), &sizesockaddr);
}

size_t Socket::Peek(void *buffer, size_t size)
{
  socklen_t sizesockaddr = sizeof(struct sockaddr_in);
  return recvfrom(sockid, buffer, size, MSG_PEEK | MSG_TRUNC, reinterpret_cast<struct sockaddr *>(&addr), &sizesockaddr);
}

int UDPServerSocket::Bind(in_port_t server_port)
{
  int retval = 0;

  addr.sin_family = AF_INET;
  addr.sin_port = htons(server_port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  /**** Create Socket ****/
  sockid = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockid == -1)
  {
    retval = errno;
    printf("Error opening socket: %d\n", retval);
    return retval;
  }

  /**** Bind Socket ****/
  retval = bind(sockid, reinterpret_cast<const struct sockaddr *>(&addr), sizeof(struct sockaddr_in));
  if (retval == -1)
  {
    retval = errno;
    printf("Error binding socket: %d\n", retval);
    Close();
    return retval;
  }

  return 0;
}

int UDPClientSocket::Bind(string server_ip, in_port_t server_port)
{
  int retval = 0;

  /**** Fill sockaddr_in structure ****/
  retval = inet_pton(AF_INET, server_ip.c_str(), &(addr.sin_addr));
  if (retval < 1)
  {
    printf("Invalid IP address\n");
    return retval;
  }
  addr.sin_family = AF_INET;
  addr.sin_port = 0;

  /**** Create Socket ****/
  sockid = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockid == -1)
  {
    retval = errno;
    printf("Error opening socket: %d\n", retval);
    return retval;
  }

  /**** Bind Socket ****/
  retval = bind(sockid, reinterpret_cast<const struct sockaddr *>(&addr), sizeof(struct sockaddr_in));
  if (retval == -1)
  {
    retval = errno;
    printf("Error binding socket: %d\n", retval);
    Close();
    return retval;
  }

  copy_hton(addr.sin_port, server_port);
  return 0;
}

void Socket::copy_ntoh(uint16_t& dest,const uint16_t src)
{
     dest=ntohs(src);
};

void Socket::copy_ntoh(uint32_t& dest,const uint32_t src)
{
     dest=ntohl(src);
};

void Socket::copy_ntoh(uint64_t& dest,const uint64_t src)
{
     dest=be64toh(src);
};

void Socket::copy_hton(uint16_t& dest,const uint16_t src)
{
     dest=htons(src);
};

void Socket::copy_hton(uint32_t& dest,const uint32_t src)
{
     dest=htonl(src);
};

void Socket::copy_hton(uint64_t& dest,const uint64_t src)
{
     dest=htobe64(src);
};