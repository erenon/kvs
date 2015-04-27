#include <cstring> // memmove
#include <algorithm>

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

bool ReadBuffer::read(boost::string_ref& string)
{
  const char* strEnd = std::find(_current, _end, '\0');
  if (strEnd == _end) { return false; }
  string = boost::string_ref(_current, strEnd - _current);
  _current = strEnd + 1;
  return true;
}

void WriteBuffer::write(const void* buffer, std::size_t size)
{
  if (_pWrite + size < _buffer.size())
  {
    memcpy(_buffer.data() + _pWrite, buffer, size);
    _pWrite += size;
  }
  else
  {
    auto newReadSize = readAvailable() + size;
    std::vector<char> newBuffer(newReadSize * 2);

    memcpy(newBuffer.data(), _buffer.data(), readAvailable());
    memcpy(newBuffer.data() + readAvailable(), buffer, size);
    _pRead = 0;
    _pWrite= newReadSize;
  }
}

const char* WriteBuffer::read()
{
  return _buffer.data() + _pRead;
}

std::size_t WriteBuffer::readAvailable() const
{
  return _pWrite - _pRead;
}

void WriteBuffer::doneRead(std::size_t size)
{
  _pRead += size;
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

void FixBuffer::enlarge()
{
  _bufferSize *= 2;
  std::unique_ptr<char[]> newBuffer(new char[_bufferSize]);
  memcpy(newBuffer.get(), read(), readAvailable());
  std::swap(_buffer, newBuffer);
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
