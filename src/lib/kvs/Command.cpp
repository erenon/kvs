#include <kvs/Command.hpp>

namespace kvs {

Command::Command(const char* header)
  :_pHeader(reinterpret_cast<const Header*>(header)),
   _pBody(header + sizeof(Header))
{}

Command::Command(const char* header, const char* body)
  :_pHeader(reinterpret_cast<const Header*>(header)),
   _pBody(body)
{}


} // namespace
