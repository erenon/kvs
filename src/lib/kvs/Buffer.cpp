#include <cstring> // memmove

#include <kvs/Buffer.hpp>

namespace kvs {

ReadBuffer::ReadBuffer(const char* buffer, std::size_t size)
  :_current(buffer),
   _end(buffer + size)
{}

bool ReadBuffer::read(char* output, const std::size_t size)
{
  if (_current + size <= _end)
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

FixWriteBuffer::FixWriteBuffer(char* buffer) :_current(buffer) {}

void FixWriteBuffer::write(const void* input, const std::size_t size)
{
  memcpy(_current, input, size);
  _current += size;
}


} // namespace
