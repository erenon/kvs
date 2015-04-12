#include <kvs/Reactor.hpp>
#include <kvs/Error.hpp>

namespace kvs {

Reactor::Reactor()
  :_epollfd(epoll_create1(0))
{
  if (! _epollfd) { failure("epoll_create1"); }
}

bool Reactor::addHandler(IOHandler* pHandler, int fd, int events)
{
  return addToEpoll(pHandler, fd, events);
}

bool Reactor::dispatch()
{
  constexpr int eventsSize = 16;
  epoll_event events[eventsSize];

  const int eventCount = epoll_wait(*_epollfd, events, eventsSize, 1000 /* 1s */);
  if (eventCount < 0) { failure("epoll_wait"); }

  for (int eventIndex = 0; eventIndex < eventCount; ++eventIndex)
  {
    IOHandler* pHandler = reinterpret_cast<IOHandler*>(events[eventIndex].data.ptr);

    if (
       (events[eventIndex].events & EPOLLERR)
    || (events[eventIndex].events & EPOLLHUP)
    )
    {
      KVS_LOG_WARNING << "ERR/HUP on handler: " << pHandler;
      removeHandler(pHandler);
    }
    else
    {
      pHandler->dispatch();
    }
  }

  return eventCount > 0;
}

bool Reactor::addToEpoll(IOHandler* pHandler, int fd, int events)
{
  epoll_event ev;
  ev.events = events;
  ev.data.ptr = pHandler;
  if (epoll_ctl(*_epollfd, EPOLL_CTL_ADD, fd, &ev) < 0)
  {
    KVS_LOG_ERROR << "Reactor: Failed to add handler: " << strerror(errno);
    return false;
  }

  return true;
}

void Reactor::addHandler(IOHandler* pHandler)
{
  std::lock_guard<std::mutex> lock(_handlersMutex);
  for (auto&& handlerPtr : _handlers)
  {
    if (handlerPtr == nullptr)
    {
      handlerPtr.reset(pHandler);
      return;
    }
  }

  _handlers.emplace_back(pHandler);
}

void Reactor::removeHandler(IOHandler* toDelete)
{
  std::lock_guard<std::mutex> lock(_handlersMutex);
  for (auto&& handlerPtr: _handlers)
  {
    if (handlerPtr.get() == toDelete)
    {
      handlerPtr.release();
      return;
    }
  }
}

} // namespace
