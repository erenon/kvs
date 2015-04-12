#ifndef KVS_COMMAND_HPP_
#define KVS_COMMAND_HPP_

#include <cstdint>

namespace kvs {

enum class CommandType : uint16_t
{
  GET,
  SET,
};

class Command
{
public:
  typedef uint16_t Size;

  constexpr Command() = default;

  constexpr Size size() const { return _size; }
  constexpr CommandType type() const { return _command; }

  // ... payload() const { ... }

private:
  Size _size;
  CommandType _command;
  // payload follows
} __attribute__((packed));

} // namespace kvs

#endif // KVS_COMMAND_HPP_
