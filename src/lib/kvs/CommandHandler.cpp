#include <kvs/CommandHandler.hpp>

namespace kvs {

CommandHandler::CommandHandler(int socket)
  :_socket(socket),
   _buffer(1 << 20)
{}

bool CommandHandler::dispatch()
{
  return !!_socket;
}


} // namespace
