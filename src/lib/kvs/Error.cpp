#include <cstdio> // perror
#include <exception> // terminate

#include <kvs/Error.hpp>

namespace kvs {

__attribute__ ((noreturn))
void failure(const char* msg)
{
  perror(msg);
  std::terminate();
}

} // namespace
