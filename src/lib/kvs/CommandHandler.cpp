#include <cstring>  // strerror

#include <kvs/CommandHandler.hpp>
#include <kvs/Command.hpp>
#include <kvs/Reactor.hpp>

namespace kvs {

CommandHandler::CommandHandler(int socket, Store& store, Reactor& reactor)
  :_socket(socket),
   _store(store),
   _buffer(1 << 20),
   _writer(_socket, reactor)
{}

bool CommandHandler::dispatch()
{
  if (_buffer.writeAvailable() < 256) { _buffer.enlarge(); }

  ssize_t rsize = read(*_socket, _buffer.write(), _buffer.writeAvailable());

  if (rsize < 0)
  {
    _socket.close();
    return false;
  }

  _buffer.doneWrite(rsize);

  while (_buffer.readAvailable() > sizeof(command::Size) + sizeof(CommandType))
  {
    ReadBuffer reader(_buffer.read(), _buffer.readAvailable());

    command::Size comSize;
    reader.read(comSize);

    if (_buffer.readAvailable() < comSize) { break; }

    CommandType comTag;
    reader.read(comTag);

    const char* comBegin = _buffer.read() + sizeof(comSize);
    auto payloadSize = comSize - sizeof(comSize);

    try
    {

      switch (comTag)
      {
      case CommandType::GET:
      {
        GetCommand input(command::deserialize{}, comBegin, payloadSize);
        SetCommand output = input.execute(_store);

        iovec serialized[SetCommand::serializedVectorSize];
        std::size_t fullSize;
        output.serialize(serialized, fullSize);
        _writer.write(serialized, SetCommand::serializedVectorSize, fullSize);

        break;
      }
      case CommandType::SET:
      {
        SetCommand input(command::deserialize{}, comBegin, payloadSize);
        input.execute(_store);
        break;
      }
      default:
      {
        KVS_LOG_ERROR << "Invalid command tag received: " << static_cast<int>(comTag);
        _socket.close();
        return false;
        break;
      }
      }

    }
    catch (const std::runtime_error& ex)
    {
      KVS_LOG_ERROR << "Failed to deserialize command";
      _socket.close();
      return false;
    }

    _buffer.doneRead(comSize);
  }

  _buffer.rewind(); // move the remaining bytes to the beginning

  return true;
}

CommandHandler::ResponseWriter::ResponseWriter(Fd& socket, Reactor& reactor)
  :_socket(socket),
   _reactor(reactor),
   _addedToReactor(false)
{}

bool CommandHandler::ResponseWriter::dispatch()
{
  ssize_t wsize = ::write(*_socket, _buffer.read(), _buffer.readAvailable());
  if (wsize < 0)
  {
    _socket.close();
    return false;
  }

  _buffer.doneRead(wsize);

  if (_buffer.readAvailable())
  {
    addToReactor();
  }

  return true;
}

void CommandHandler::ResponseWriter::write(iovec* output, std::size_t vecSize, std::size_t fullSize)
{
  // try drain buffer if any
  ssize_t wsize;
  if (_buffer.readAvailable())
  {
    wsize = ::write(*_socket, _buffer.read(), _buffer.readAvailable());
    if (wsize < 0)
    {
      KVS_LOG_WARNING << "CommandHandler resp write: " << strerror(errno);
      _socket.close();
      return;
    }

    _buffer.doneRead(wsize);
  }

  if (_buffer.readAvailable())
  {
    // add everything to backlog
    writeBuffer(output, vecSize);
  }
  else
  {
    // write latest
    wsize = writev(*_socket, output, vecSize);
    if (wsize < 0)
    {
      KVS_LOG_WARNING << "CommandHandler resp write: " << strerror(errno);
      _socket.close();
      return;
    }

    std::size_t uwsize = wsize;
    if (uwsize == fullSize) { return; }

    KVS_LOG_DEBUG << "CommandHandler output socket was full: " << uwsize << "/" << fullSize;

    // save the rest
    std::size_t i = 0;
    for (; i < vecSize; ++i)
    {
      if (uwsize >= output[i].iov_len)
      {
        uwsize -= output[i].iov_len;
      }
      else if (uwsize)
      {
        const char* chunk = reinterpret_cast<const char*>(output[i].iov_base) + uwsize;
        _buffer.write(chunk, output[i].iov_len - uwsize);
        break;
      }
    }
    writeBuffer(output + i, vecSize - i);
  }

  if (_buffer.readAvailable())
  {
    addToReactor();
  }
}

void CommandHandler::ResponseWriter::writeBuffer(iovec* output, std::size_t vecSize)
{
  for (std::size_t i = 0; i < vecSize; ++i)
  {
    _buffer.write(output[i].iov_base, output[i].iov_len);
  }
}

void CommandHandler::ResponseWriter::addToReactor()
{
  if (_addedToReactor)
  {
    _reactor.readdHandler(this, *_socket, EPOLLOUT | EPOLLONESHOT);
  }
  else
  {
    _addedToReactor = _reactor.addHandler(this, *_socket, EPOLLOUT | EPOLLONESHOT);
  }
}

} // namespace
