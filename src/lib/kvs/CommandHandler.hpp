#ifndef KVS_COMMANDHANDLER_HPP_
#define KVS_COMMANDHANDLER_HPP_

#include <sys/uio.h>

#include <kvs/IOHandler.hpp>
#include <kvs/Fd.hpp>
#include <kvs/Buffer.hpp>

namespace kvs {

class Store;
class Reactor;

class CommandHandler : public IOHandler
{
public:
  CommandHandler(int socket, Store& store, Reactor& reactor);

  bool dispatch() override;

private:
  class ResponseWriter : public IOHandler
  {
  public:
    ResponseWriter(int socket, Reactor& reactor);

    bool dispatch() override;

    void write(iovec* output, std::size_t vecSize, std::size_t fullSize);

  private:
    void writeBuffer(iovec* output, std::size_t vecSize);
    void addToReactor();

    int _socket;
    Reactor& _reactor;
    WriteBuffer _buffer;
    bool _addedToReactor;
  };

  Fd _socket;
  Store& _store;
  FixBuffer _buffer;
  ResponseWriter _writer;
};

} // namespace kvs

#endif // KVS_COMMANDHANDLER_HPP_
