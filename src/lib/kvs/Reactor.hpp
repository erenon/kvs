#ifndef KVS_REACTOR_HPP_
#define KVS_REACTOR_HPP_

#include <vector>
#include <memory>
#include <mutex>
#include <map>
#include <atomic>

#include <sys/epoll.h>

#include <kvs/Fd.hpp>
#include <kvs/IOHandler.hpp>

namespace kvs {

class Reactor
{
public:
  Reactor();

  template <typename Handler, typename... HandlerArgs>
  bool addHandler(int fd, int events, HandlerArgs&&... handlerArgs);

  bool addHandler(IOHandler* pHandler, int fd, int events);
  bool readdHandler(IOHandler* pHandler, int fd, int events);

  bool dispatch();

  bool isStopped() const { return _stopped.load(); }
  void stop() { _stopped.store(true); }

private:
  void addHandler(IOHandler* pHandler);
  bool addToEpoll(IOHandler* pHandler, int fd, int events);
  bool removeFromEpoll(IOHandler* pHandler);
  void removeHandler(IOHandler* toDelete);

  std::atomic<bool> _stopped;
  Fd _epollfd;
  std::mutex _handlersMutex;
  std::vector<std::unique_ptr<IOHandler>> _handlers;
  std::mutex _fdsMutex;
  std::map<IOHandler*, int> _fdHandlers;
};

template <typename Handler, typename... HandlerArgs>
bool Reactor::addHandler(int fd, int events, HandlerArgs&&... handlerArgs)
{
  IOHandler* pHandler = new Handler(std::forward<HandlerArgs>(handlerArgs)...);
  addHandler(pHandler);

  return addToEpoll(pHandler, fd, events);
}

} // namespace kvs

#endif // KVS_REACTOR_HPP_
