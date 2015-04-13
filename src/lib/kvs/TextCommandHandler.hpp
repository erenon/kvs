#ifndef KVS_TEXTCOMMANDHANDLER_HPP_
#define KVS_TEXTCOMMANDHANDLER_HPP_

#include <kvs/IOHandler.hpp>
#include <kvs/Fd.hpp>

namespace kvs {

class Reactor;

class TextCommandHandler : public IOHandler
{
public:
  TextCommandHandler(int fd, Reactor& reactor)
    :_fd(fd),
     _reactor(reactor)
  {}

  bool dispatch() override;

private:
  Fd _fd;
  Reactor& _reactor;
};

} // namespace kvs

#endif // KVS_TEXTCOMMANDHANDLER_HPP_
