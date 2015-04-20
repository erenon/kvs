#ifndef KVS_TEXTCOMMANDHANDLER_HPP_
#define KVS_TEXTCOMMANDHANDLER_HPP_

#include <kvs/IOHandler.hpp>
#include <kvs/Fd.hpp>

namespace kvs {

class Reactor;
class Store;

// TODO rename to ConsoleCommandHandler
class TextCommandHandler : public IOHandler
{
public:
  TextCommandHandler(
    int in,
    int out,
    Store& store,
    Reactor& reactor
  );

  bool dispatch() override;

private:
  const char* processCommand(const char* buffer, const char* end);

  int _in;
  int _out;
  Store& _store;
  Reactor& _reactor;
};

} // namespace kvs

#endif // KVS_TEXTCOMMANDHANDLER_HPP_
