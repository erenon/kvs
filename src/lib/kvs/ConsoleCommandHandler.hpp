#ifndef KVS_CONSOLECOMMANDHANDLER_HPP_
#define KVS_CONSOLECOMMANDHANDLER_HPP_

#include <kvs/IOHandler.hpp>

namespace kvs {

class Reactor;
class Store;

class ConsoleCommandHandler : public IOHandler
{
public:
  ConsoleCommandHandler(
    int in,
    int out,
    Store& store,
    Reactor& reactor
  );

  ~ConsoleCommandHandler();

  bool dispatch() override;

private:
  const char* processCommand(const char* buffer, const char* end);

  int _in;
  int _out;
  Store& _store;
  Reactor& _reactor;
};

} // namespace kvs

#endif // KVS_CONSOLECOMMANDHANDLER_HPP_
