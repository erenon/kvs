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
  typedef std::size_t Size;

  Command(const char* header);
  Command(const char* header, const char* body);

  Size size() const
  {
    return _pHeader->size;
  }

  CommandType type() const
  {
    return _pHeader->type;
  }

  const char* payload() const
  {
    return _pBody;
  }

private:
  struct Header
  {
    const Size size;
    const CommandType type;
  } __attribute__((packed));

  const Header* _pHeader;
  const char* _pBody;
};

} // namespace kvs

#endif // KVS_COMMAND_HPP_
