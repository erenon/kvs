#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#include <kvs/Connection.hpp>

namespace kvs {

Connection::Connection(const char* serverIp, int serverPort)
{
  sockaddr_in serverAddr;

  if (! (_serverConn = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)))
  {
    failure("Connection socket");
  }

  // Setup address
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET; // IP
  serverAddr.sin_port = htons(serverPort);

  if (! inet_aton(serverIp, &serverAddr.sin_addr))
  {
    failure("Connection inet_aton");
  }

  if (connect(*_serverConn, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0)
  {
    failure("Connection connect");
  }
}

void Connection::pop(const Key& key)
{
  PopCommand req(key);
  sendCommand(req);
}

} // namespace kvs
