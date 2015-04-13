#include <climits>
#include <iostream>
#include <cstring> // strerror

#include <kvs/TextCommandHandler.hpp>
#include <kvs/Reactor.hpp>
#include <kvs/Log.hpp>

namespace kvs {

bool TextCommandHandler::dispatch()
{
  char buff[LINE_MAX];
  ssize_t rsize = read(*_fd, buff, LINE_MAX);

  if (rsize > 0)
  {
    if (buff[rsize - 1] == '\n')
    {
      std::cout.write(buff, rsize);
    }
    else
    {
      KVS_LOG_WARNING << "TextCommandHandler: input too long";
      // consume all
      while (rsize == LINE_MAX)
      {
        rsize = read(*_fd, buff, LINE_MAX);
      }
    }
  }
  else if (rsize == 0)
  {
    _reactor.stop();
    return false;
  }
  else
  {
    KVS_LOG_ERROR << "TextCommandHandler read error: " << strerror(errno);
    return false;
  }

  return true;
}

} // namespace
