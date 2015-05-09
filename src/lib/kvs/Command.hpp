#ifndef KVS_COMMAND_HPP_
#define KVS_COMMAND_HPP_

#include <cstdint>
#include <memory>
#include <sys/uio.h>

#include <boost/utility/string_ref.hpp>

#include <kvs/Buffer.hpp>

namespace kvs {

typedef boost::string_ref Key;

namespace command {

typedef uint64_t Size;

enum class Tag : uint16_t
{
  GET,
  SET,
  PUSH,
  POP,
  SUM,
  MAX,
  MIN,
};

struct deserialize {};

} // namespace command

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

  SetCommand(command::deserialize, const char* buffer, command::Size size);

  void execute(Store& store) const;
  std::pair<const char*, std::size_t> value() const;

  static constexpr int serializedVectorSize = 5;

  void serialize(iovec* output, command::Size& size) const;

private:
  static const command::Tag _tag;

  Key _key;
  std::size_t _serializedValueSize;
  const char* _serializedValue;
};

class GetCommand
{
public:
  GetCommand(const Key& key) : _key(key) {}

  GetCommand(command::deserialize, const char* buffer, command::Size size);

  SetCommand execute(const Store& store) const;

  static constexpr int serializedVectorSize = 3;

  void serialize(iovec* output, command::Size& size) const;

private:
  static const command::Tag _tag;

  Key _key;
};

class PushCommand
{
public:
  PushCommand(
    const Key& key,
    std::size_t serializedValueSize,
    const char* serializedValue
  )
    :_key(key),
     _serializedValueSize(serializedValueSize),
     _serializedValue(serializedValue)
  {}

  PushCommand(command::deserialize, const char* buffer, command::Size size);

  void execute(Store& store) const;
  std::pair<const char*, std::size_t> value() const;

  static constexpr int serializedVectorSize = 5;

  void serialize(iovec* output, command::Size& size) const;

private:
  static const command::Tag _tag;

  Key _key;
  std::size_t _serializedValueSize;
  const char* _serializedValue;
};

class PopCommand
{
public:
  PopCommand(const Key& key) : _key(key) {}

  PopCommand(command::deserialize, const char* buffer, command::Size size);

  void execute(Store& store) const;

  static constexpr int serializedVectorSize = 3;

  void serialize(iovec* output, command::Size& size) const;

private:
  static const command::Tag _tag;

  Key _key;
};

class SumCommand
{
public:
  SumCommand(const Key& key) : _key(key) {}

  SumCommand(command::deserialize, const char* buffer, command::Size size);

  SetCommand execute(const Store& store) const;

  static constexpr int serializedVectorSize = 3;

  void serialize(iovec* output, command::Size& size) const;

private:
  static const command::Tag _tag;

  Key _key;
};

class MaxCommand
{
public:
  MaxCommand(const Key& key) : _key(key) {}

  MaxCommand(command::deserialize, const char* buffer, command::Size size);

  SetCommand execute(const Store& store) const;

  static constexpr int serializedVectorSize = 3;

  void serialize(iovec* output, command::Size& size) const;

private:
  static const command::Tag _tag;

  Key _key;
};

class MinCommand
{
public:
  MinCommand(const Key& key) : _key(key) {}

  MinCommand(command::deserialize, const char* buffer, command::Size size);

  SetCommand execute(const Store& store) const;

  static constexpr int serializedVectorSize = 3;

  void serialize(iovec* output, command::Size& size) const;

private:
  static const command::Tag _tag;

  Key _key;
};

} // namespace kvs

#endif // KVS_COMMAND_HPP_
