#ifndef KVS_COMMANDHANDLER_HPP_
#define KVS_COMMANDHANDLER_HPP_

#include <kvs/IOHandler.hpp>
#include <kvs/Fd.hpp>
#include <kvs/Buffer.hpp>

namespace kvs {

class CommandHandler : public IOHandler
{
public:
  CommandHandler(int socket);

  bool dispatch() override;

private:
  Fd _socket;
  FixBuffer _buffer;
};

} // namespace kvs

#endif // KVS_COMMANDHANDLER_HPP_
