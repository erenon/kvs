#include <cstring> // memmove

#include <kvs/Buffer.hpp>

namespace kvs {

ReadBuffer::ReadBuffer(const char* buffer, std::size_t size)
  :_current(buffer),
   _end(buffer + size)
{}

bool ReadBuffer::read(char* output, std::size_t size)
{
  std::size_t readAvail = _end - _current;
  if (readAvail >= size)
  {
    memcpy(output, _current, size);
    _current += size;
    return true;
  }

  return false;
}

FixBuffer::FixBuffer(std::size_t size)
  :_bufferSize(size),
   _buffer(new char[size]),
   _pWrite(0),
   _pRead(0)
{}

std::size_t FixBuffer::writeAvailable() const
{
  return _bufferSize - _pWrite;
}

std::size_t FixBuffer::readAvailable() const
{
  return _pWrite - _pRead;
}

char* FixBuffer::write()
{
  return _buffer.get() + _pWrite;
}

const char* FixBuffer::read() const
{
  return _buffer.get() + _pRead;
}

void FixBuffer::doneWrite(std::size_t size)
{
  _pWrite += size;
}

void FixBuffer::doneRead(std::size_t size)
{
  _pRead += size;
}

void FixBuffer::rewind()
{
  memmove(_buffer.get(), read(), readAvailable());
  _pWrite = readAvailable();
  _pRead = 0;
}

} // namespace
