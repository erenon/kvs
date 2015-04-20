#ifndef KVS_COMMAND_HPP_
#define KVS_COMMAND_HPP_

#include <cstdint>
#include <memory>

#include <boost/utility/string_ref.hpp>

#include <kvs/Buffer.hpp>

namespace kvs {

typedef boost::string_ref Key;

enum class CommandType : uint16_t
{
  GET,
  SET,
};

class Store;

class SetCommand
{
public:
  SetCommand(
    const Key& key,
    std::size_t serializedValueSize,
    const char* serializedValue
  )
    :_key(key),
     _serializedValueSize(serializedValueSize),
     _serializedValue(serializedValue)
  {}

  void execute(Store& store) const;
  std::pair<const char*, std::size_t> value() const;

private:
  Key _key;
  std::size_t _serializedValueSize;
  const char* _serializedValue;
};

class GetCommand
{
public:
  GetCommand(const Key& key) : _key(key) {}

  SetCommand execute(Store& store) const;

private:
  Key _key;
};

} // namespace kvs

#endif // KVS_COMMAND_HPP_
