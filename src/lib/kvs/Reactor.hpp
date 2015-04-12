#ifndef KVS_REACTOR_HPP_
#define KVS_REACTOR_HPP_

#include <vector>
#include <memory>
#include <mutex>

#include <sys/epoll.h>

#include <kvs/Fd.hpp>
#include <kvs/IOHandler.hpp>
#include <kvs/Log.hpp>

namespace kvs {

class Reactor
{
public:
  Reactor();

  template <typename Handler, typename... HandlerArgs>
  bool addHandler(int fd, int events, HandlerArgs... handlerArgs);

  bool addHandler(IOHandler* pHandler, int fd, int events);

  bool dispatch();

private:
  void addHandler(IOHandler* pHandler);
  bool addToEpoll(IOHandler* pHandler, int fd, int events);
  void removeHandler(IOHandler* toDelete);

  Fd _epollfd;
  std::mutex _handlersMutex;
  std::vector<std::unique_ptr<IOHandler>> _handlers;
};

template <typename Handler, typename... HandlerArgs>
bool Reactor::addHandler(int fd, int events, HandlerArgs... handlerArgs)
{
  IOHandler* pHandler = new Handler(std::forward<HandlerArgs>(handlerArgs)...);
  addHandler(pHandler);

  return addToEpoll(pHandler, fd, events);
}

} // namespace kvs

#endif // KVS_REACTOR_HPP_
