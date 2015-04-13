#ifndef KVS_BUFFER_HPP_
#define KVS_BUFFER_HPP_

#include <memory>

namespace kvs {

class ReadBuffer
{
public:
  ReadBuffer(const char* buffer, std::size_t size);

  bool read(char* output, std::size_t size);

  template <typename Field>
  bool read(Field& output)
  {
    return read(reinterpret_cast<char*>(&output), sizeof(Field));
  }

private:
  const char* _current;
  const char* _end;
};

class FixBuffer
{
public:
  FixBuffer(std::size_t size);

  std::size_t writeAvailable() const;
  std::size_t readAvailable() const;

  char* write();
  const char* read() const;

  void doneWrite(std::size_t size);
  void doneRead(std::size_t size);

  void rewind();

private:
  const std::size_t _bufferSize;
  std::unique_ptr<char[]> _buffer;
  std::size_t _pWrite;
  std::size_t _pRead;
};

} // namespace kvs

#endif // KVS_BUFFER_HPP_
