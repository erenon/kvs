#ifndef KVS_BUFFER_HPP_
#define KVS_BUFFER_HPP_

namespace kvs {

class ReadBuffer
{
public:
  ReadBuffer(const char* buffer, std::size_t size)
    :_current(buffer),
     _end(buffer + size)
  {}

  bool read(char* output, std::size_t size)
  {
    if (_end - _current >= size)
    {
      memcpy(output, _current, size);
      _current += size;
      return true;
    }

    return false;
  }

  template <typename Field>
  bool read(Field& output)
  {
    return read(reinterpret_cast<char*>(&output), sizeof(Field));
  }

private:
  const char* _current;
  const char* _end;
};

} // namespace kvs

#endif // KVS_BUFFER_HPP_
