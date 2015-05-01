#ifndef KVS_BUFFER_HPP_
#define KVS_BUFFER_HPP_

#include <memory>
#include <vector>

#include <boost/utility/string_ref.hpp>

namespace kvs {

class ReadBuffer
{
public:
  ReadBuffer(const char* buffer, std::size_t size);

  bool read(char* output, const std::size_t size);

  template <typename Field>
  bool read(Field& output)
  {
    return read(reinterpret_cast<char*>(&output), sizeof(Field));
  }

  bool read(boost::string_ref& string);

  const char* get() { return _current; }

  explicit operator bool() const { return _current != _end; }

  std::size_t size() const { return _end - _current; }

  void discard(std::size_t size) { _current += size; }

private:
  const char* _current;
  const char* _end;
};

class WriteBuffer
{
public:
  WriteBuffer() = default;

  void write(const void* buffer, std::size_t size);

  const char* read();
  std::size_t readAvailable() const;
  void doneRead(std::size_t size);

private:
  std::vector<char> _buffer;
  std::size_t _pRead = 0;
  std::size_t _pWrite = 0;
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

  void enlarge();

private:
  std::size_t _bufferSize;
  std::unique_ptr<char[]> _buffer;
  std::size_t _pWrite;
  std::size_t _pRead;
};

class FixWriteBuffer
{
public:
  FixWriteBuffer(char* buffer);

  void write(const void* input, const std::size_t size);

  template <typename Field>
  void write(const Field& input)
  {
    write(reinterpret_cast<const void*>(&input), sizeof(Field));
  }

private:
  char* _current;
};

} // namespace kvs

#endif // KVS_BUFFER_HPP_
