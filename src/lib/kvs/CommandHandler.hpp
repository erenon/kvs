#ifndef KVS_COMMANDHANDLER_HPP_
#define KVS_COMMANDHANDLER_HPP_

#include <kvs/IOHandler.hpp>
#include <kvs/Fd.hpp>

namespace kvs {

class CommandHandler : public IOHandler
{
public:
  CommandHandler(int socket) :_socket(socket) {}

  bool dispatch() override { return true; }

private:
  Fd _socket;
};

} // namespace kvs

#endif // KVS_COMMANDHANDLER_HPP_
