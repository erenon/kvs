#include <cstring> // memcpy

#include <kvs/Command.hpp>
#include <kvs/Store.hpp>
#include <kvs/Value.hpp>

namespace kvs {

void SetCommand::execute(Store& store) const
{
  // TODO write persistent store

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

SetCommand GetCommand::execute(Store& store) const
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

} // namespace
