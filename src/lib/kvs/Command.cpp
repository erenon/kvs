#include <cstring> // memcpy

#include <kvs/Command.hpp>
#include <kvs/Store.hpp>
#include <kvs/Value.hpp>
#include <kvs/Buffer.hpp>
#include <kvs/Error.hpp>

namespace kvs {

const command::Tag GetCommand::_tag = command::Tag::GET;
const command::Tag SetCommand::_tag = command::Tag::SET;

SetCommand::SetCommand(command::deserialize, const char* buffer, command::Size size)
{
  ReadBuffer reader(buffer, size);

  command::Tag actualTag;

  check(reader.read(actualTag));
  check(actualTag == _tag);
  check(reader.read(_key));
  check(reader.read(_serializedValueSize));
  _serializedValue = reader.get();
}

void SetCommand::execute(Store& store) const
{
  // write persistent store
  {
    iovec serialized[serializedVectorSize];
    std::size_t fullSize;
    serialize(serialized, fullSize);
    store.writePersStore(serialized, serializedVectorSize, fullSize);
  }

  auto&& entry = store[_key];
  if (entry.first < _serializedValueSize)
  {
    entry.second.reset(new char[_serializedValueSize]);
  }

  memcpy(entry.second.get(), _serializedValue, _serializedValueSize);
  entry.first = _serializedValueSize;
}

std::pair<const char*, std::size_t> SetCommand::value() const
{
  return {_serializedValue, _serializedValueSize};
}

void SetCommand::serialize(iovec* output, command::Size& size) const
{
  size = sizeof(size) + sizeof(_tag) + _key.size() + 1 + sizeof(_serializedValueSize) + _serializedValueSize;

  output[0].iov_base = &size;
  output[0].iov_len = sizeof(size);

  output[1].iov_base = const_cast<command::Tag*>(&_tag);
  output[1].iov_len = sizeof(_tag);

  output[2].iov_base = const_cast<char*>(_key.data());
  output[2].iov_len = _key.size() + 1;

  output[3].iov_base = const_cast<std::size_t*>(&_serializedValueSize);
  output[3].iov_len = sizeof(_serializedValueSize);

  output[4].iov_base = const_cast<char*>(_serializedValue);
  output[4].iov_len = _serializedValueSize;
}

GetCommand::GetCommand(command::deserialize, const char* buffer, command::Size size)
{
  ReadBuffer reader(buffer, size);
  command::Tag actualTag;

  check(reader.read(actualTag));
  check(actualTag == _tag);
  check(reader.read(_key));
}

SetCommand GetCommand::execute(const Store& store) const
{
  auto finder = store.find(_key);
  if (finder != store.end())
  {
    // *finder is the requested value
    SetCommand result(_key, finder->second.first, finder->second.second.get());
    return result;
  }
  else
  {
    // not found
    SetCommand result(_key, NullValue::serializedSize, NullValue::serializedValue());
    return result;
  }
}

void GetCommand::serialize(iovec* output, command::Size& size) const
{
  size = sizeof(size) + sizeof(_tag) + _key.size() + 1;

  output[0].iov_base = &size;
  output[0].iov_len = sizeof(size);

  output[1].iov_base = const_cast<command::Tag*>(&_tag);
  output[1].iov_len = sizeof(_tag);

  output[2].iov_base = const_cast<char*>(_key.data());
  output[2].iov_len = _key.size() + 1;
}

} // namespace
