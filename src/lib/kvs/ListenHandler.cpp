#include <cstring> // memset
#include <cerrno>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#include <kvs/ListenHandler.hpp>
#include <kvs/Error.hpp>
#include <kvs/CommandHandler.hpp>

namespace kvs {

ListenHandler::ListenHandler(Reactor& reactor, in_port_t port)
  :_reactor(reactor)
{
  // open listening socket

  _listenSocket = socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
  if (! _listenSocket) { failure("socket"); }

  sockaddr_in server;
  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = port;

  if (bind(*_listenSocket, (sockaddr*)&server, sizeof(server)) < 0)
  {
    failure("bind");
  }

  if (::listen(*_listenSocket, SOMAXCONN) < 0)
  {
    failure("listen");
  }

  // Add to reactor
  if (! _reactor.addHandler(this, *_listenSocket, EPOLLIN))
  {
    failure("addHandler");
  }
}

bool ListenHandler::dispatch()
{
  int client = 0;

  do
  {
    client = accept4(*_listenSocket, nullptr, nullptr, SOCK_NONBLOCK);
    if (client >= 0)
    {
      _reactor.addHandler<CommandHandler>(client, EPOLLIN, client);
    }
    else
    {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
      {
        break;
      }
      else
      {
        perror("accept4");
        return false;
      }
    }
  } while (client >= 0);

  return true;
}

} // namespace
