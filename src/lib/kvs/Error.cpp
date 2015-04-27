#include <cstdio> // perror
#include <exception> // terminate
#include <stdexcept>

#include <kvs/Error.hpp>

namespace kvs {

__attribute__ ((noreturn))
void failure(const char* msg)
{
  perror(msg);
  std::terminate();
}

void check(bool ok)
{
  if (!ok)
  {
    throw std::runtime_error("Check failed");
  }
}

} // namespace
