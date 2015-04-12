#ifndef KVS_FD_HPP_
#define KVS_FD_HPP_

#include <unistd.h>

namespace kvs {

class Fd
{
public:
  typedef int (*CloseFn)(int);

  Fd() {}
  Fd(int fd) : _fd(fd) {}
  Fd(int fd, CloseFn close) : _fd(fd), _close(close) {}

  Fd(const Fd&) = delete;
  void operator=(const Fd&) = delete;

  Fd(Fd&& rhs)
    :Fd(rhs._fd, rhs._close)
  {
    rhs._fd = -1;
  }

  Fd& operator=(Fd&& rhs)
  {
    if (this != &rhs)
    {
      close();
      _fd = rhs._fd;
      _close = rhs._close;
      rhs._fd = -1;
    }

    return *this;
  }

  ~Fd() { close(); }

  void close()
  {
    if (_fd >= 0)
    {
      _close(_fd);
      _fd = -1;
    }
  }

  explicit operator bool() const
  {
    return _fd >= 0;
  }

  int operator*() const
  {
    return _fd;
  }

private:
  int _fd = -1;
  CloseFn _close = ::close;
};

inline bool operator<(const Fd& a, const Fd& b)
{
  return *a < *b;
}

inline bool operator<(int a, const Fd& b)
{
  return a < b;
}

inline bool operator==(const Fd& a, int b)
{
  return *a == b;
}

} // namespace kvs

#endif // KVS_FD_HPP_
