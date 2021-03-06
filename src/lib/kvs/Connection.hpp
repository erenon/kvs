#ifndef KVS_CONNECTION_HPP_
#define KVS_CONNECTION_HPP_

#include <memory>

#include <sys/uio.h> // writev
#include <sys/socket.h> // recv

#include <boost/utility/string_ref.hpp>

#include <kvs/Fd.hpp>
#include <kvs/Error.hpp>
#include <kvs/Command.hpp>
#include <kvs/Value.hpp>

namespace kvs {

class Connection
{
public:
  typedef boost::string_ref Key;

  Connection(const char* serverIp, int serverPort);

  template <typename Field>
  bool get(const Key& key, Field& result);

  template <typename Field>
  void set(const Key& key, const Field& value);

  template <typename Field>
  void push(const Key& key, const Field& value);

  void pop(const Key& key);

  template <typename Field>
  bool sum(const Key& key, Field& result);

  template <typename Field>
  bool max(const Key& key, Field& result);

  template <typename Field>
  bool min(const Key& key, Field& result);

  void source(const Key& library);

  void execute(const Key& procedure);

private:
  template <typename Command>
  void sendCommand(const Command& command);

  template <typename Command>
  Command recvCommand();

  Fd _serverConn;

  std::size_t _recvBufferSize = 0;
  std::unique_ptr<char[]> _recvBuffer;

  std::size_t _sendBufferSize = 0;
  std::unique_ptr<char[]> _sendBuffer;
};

template <typename Field>
bool Connection::get(const Key& key, Field& result)
{
  GetCommand req(key);
  sendCommand(req);

  SetCommand resp = recvCommand<SetCommand>();

  auto value = resp.value();
  TypedValue tvalue = value::deserialize(value.first, value.second);

  Field* pResult = boost::get<Field>(&tvalue);
  if (!pResult) { return false; }

  result = *pResult;
  return true;
}

template <typename Field>
void Connection::set(const Key& key, const Field& value)
{
  auto serSize = value::serializedSize(value);
  if (_sendBufferSize < serSize)
  {
    _sendBufferSize = serSize;
    _sendBuffer.reset(new char[_sendBufferSize]);
  }

  value::serialize(value, _sendBuffer.get());

  SetCommand req(key, serSize, _sendBuffer.get());
  sendCommand(req);
}


template <typename Field>
void Connection::push(const Key& key, const Field& value)
{
  auto serSize = value::serializedSize(value);
  if (_sendBufferSize < serSize)
  {
    _sendBufferSize = serSize;
    _sendBuffer.reset(new char[_sendBufferSize]);
  }

  value::serialize(value, _sendBuffer.get());

  PushCommand req(key, serSize, _sendBuffer.get());
  sendCommand(req);
}

template <typename Field>
bool Connection::sum(const Key& key, Field& result)
{
  SumCommand req(key);
  sendCommand(req);

  SetCommand resp = recvCommand<SetCommand>();

  auto value = resp.value();
  TypedValue tvalue = value::deserialize(value.first, value.second);

  Field* pResult = boost::get<Field>(&tvalue);
  if (!pResult) { return false; }

  result = *pResult;
  return true;
}

template <typename Field>
bool Connection::max(const Key& key, Field& result)
{
  MaxCommand req(key);
  sendCommand(req);

  SetCommand resp = recvCommand<SetCommand>();

  auto value = resp.value();
  TypedValue tvalue = value::deserialize(value.first, value.second);

  Field* pResult = boost::get<Field>(&tvalue);
  if (!pResult) { return false; }

  result = *pResult;
  return true;
}

template <typename Field>
bool Connection::min(const Key& key, Field& result)
{
  MinCommand req(key);
  sendCommand(req);

  SetCommand resp = recvCommand<SetCommand>();

  auto value = resp.value();
  TypedValue tvalue = value::deserialize(value.first, value.second);

  Field* pResult = boost::get<Field>(&tvalue);
  if (!pResult) { return false; }

  result = *pResult;
  return true;
}

template <typename Command>
void Connection::sendCommand(const Command& command)
{
  enum { vecSize = Command::serializedVectorSize };
  iovec input[vecSize];
  command::Size csize = 0;
  command.serialize(input, csize);

  ssize_t wsize = writev(*_serverConn, input, vecSize);

  if (wsize < csize) { failure("Connection writev"); }
}

template <typename Command>
Command Connection::recvCommand()
{
  command::Size csize = 0;
  ssize_t rsize = recv(*_serverConn, &csize, sizeof(csize), MSG_WAITALL);

  if (rsize != sizeof(csize)) { failure("Connection header recv"); }

  if (csize > _recvBufferSize)
  {
    _recvBufferSize = csize;
    _recvBuffer.reset(new char[_recvBufferSize]);
  }

  if (csize < sizeof(csize)) { failure("Connection: invalid cszie received //"); }

  const auto payloadSize = csize - sizeof(csize);
  rsize = recv(*_serverConn, _recvBuffer.get(), payloadSize, MSG_WAITALL);

  if (rsize != payloadSize) { failure("Connection body recv"); }

  return Command{command::deserialize{}, _recvBuffer.get(), payloadSize};
}

} // namespace kvs

#endif // KVS_CONNECTION_HPP_
