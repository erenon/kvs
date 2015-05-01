#include <cstring> // memset
#include <cerrno>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#include <kvs/ListenHandler.hpp>
#include <kvs/Error.hpp>
#include <kvs/CommandHandler.hpp>
#include <kvs/Log.hpp>

namespace kvs {

ListenHandler::ListenHandler(Reactor& reactor, uint16_t port, Store& store)
  :_reactor(reactor),
   _store(store)
{
  // open listening socket

  _listenSocket = socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
  if (! _listenSocket) { failure("socket"); }

#ifdef SO_REUSEPORT
  int optval = 1;
  setsockopt(*_listenSocket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
#endif

  sockaddr_in server;
  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(port);

  if (bind(*_listenSocket, (sockaddr*)&server, sizeof(server)) < 0)
  {
    failure("bind");
  }

  if (listen(*_listenSocket, SOMAXCONN) < 0)
  {
    failure("listen");
  }

  // Add to reactor
  if (! _reactor.addHandler(this, *_listenSocket, EPOLLIN))
  {
    failure("addHandler");
  }

  KVS_LOG_INFO << "Listening at port: " << port;
}

bool ListenHandler::dispatch()
{
  int client = 0;

  do
  {
    client = accept4(*_listenSocket, nullptr, nullptr, SOCK_NONBLOCK);
    if (client >= 0)
    {
      if (_reactor.addHandler<CommandHandler>(
        client, EPOLLIN,
        client, _store, _reactor
      ))
      {
        KVS_LOG_INFO << "Client accepted";
      }
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
