#ifndef KVS_LISTENHANDLER_HPP_
#define KVS_LISTENHANDLER_HPP_

#include <cstdint>

#include <kvs/IOHandler.hpp>
#include <kvs/Fd.hpp>
#include <kvs/Reactor.hpp>

namespace kvs {

class Store;

class ListenHandler : public IOHandler
{
public:
  ListenHandler(Reactor& reactor, uint16_t port, Store& store);

  bool dispatch() override;

private:
  Reactor& _reactor;
  Fd _listenSocket;
  Store& _store;
};

} // namespace kvs

#endif // KVS_LISTENHANDLER_HPP_
