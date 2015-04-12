#ifndef KVS_TEXTCOMMANDHANDLER_HPP_
#define KVS_TEXTCOMMANDHANDLER_HPP_

#include <kvs/IOHandler.hpp>

namespace kvs {

class TextCommandHandler : public IOHandler
{
public:
  TextCommandHandler(int fd) : _fd(fd) {}

  bool dispatch() override;

private:
  int _fd;
};

} // namespace kvs

#endif // KVS_TEXTCOMMANDHANDLER_HPP_
