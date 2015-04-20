#include <stdexcept>
#include <cstring> // strerror
#include <fcntl.h>

#include <kvs/Store.hpp>

namespace kvs {

Store::Store(const char* persStore)
{
  if (persStore)
  {
    _persStore = open(persStore, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP);

    if (! _persStore)
    {
      throw std::runtime_error(std::string("Failed to open persistent store: ") + persStore);
    }

    // TODO read persistent storage, execute commands

    if (lseek(*_persStore, 0, SEEK_END) == off_t(-1))
    {
      throw std::runtime_error(std::string("Failed to seek in persistent store: ") + strerror(errno));
    }
  }
}

void Store::writePersStore(const char* command, const std::size_t size)
{
  if (_persStore)
  {
    ssize_t wsize = write(*_persStore, command, size);
    ssize_t expected = size;
    if (wsize < expected)
    {
      throw std::runtime_error(std::string("Failed to write persistent store: ") + strerror(errno));
    }
  } // else: persistent storage was turned off
}

void Store::writePersStore(const iovec* pIovec, const std::size_t vecSize)
{
  if (_persStore)
  {
    ssize_t wsize = writev(*_persStore, pIovec, vecSize);

    ssize_t expected = 0;
    for (std::size_t i = 0; i < vecSize; ++i)
    {
      expected += pIovec[i].iov_len;
    }

    if (wsize < expected)
    {
      throw std::runtime_error(std::string("Failed to write persistent store: ") + strerror(errno));
    }
  } // else: persistent storage was turned off
}

Store::Container::mapped_type& Store::operator[](const std::string& key)
{
  return _store[key];
}

Store::Container::iterator Store::find(const std::string& key)
{
  return _store.find(key);
}

Store::Container::iterator Store::end()
{
  return _store.end();
}

} // namespace kvs
