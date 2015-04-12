#ifndef KVS_LISTENHANDLER_HPP_
#define KVS_LISTENHANDLER_HPP_

#include <arpa/inet.h> // in_port_t

#include <kvs/IOHandler.hpp>
#include <kvs/Fd.hpp>
#include <kvs/Reactor.hpp>

namespace kvs {

class ListenHandler : public IOHandler
{
public:
  ListenHandler(Reactor& reactor, in_port_t port);

  bool dispatch() override;

private:
  Reactor& _reactor;
  Fd _listenSocket;
};

} // namespace kvs

#endif // KVS_LISTENHANDLER_HPP_
