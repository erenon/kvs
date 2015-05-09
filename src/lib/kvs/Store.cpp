#include <stdexcept>
#include <cstring> // strerror
#include <fcntl.h>

#include <sys/mman.h>

#include <kvs/Store.hpp>

namespace kvs {

Store::Store(const char* persStore)
{
  if (persStore)
  {
    Fd prevStore(open(persStore, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP));

    if (! prevStore)
    {
      throw std::runtime_error(std::string("Failed to open persistent store: ") + persStore);
    }

    // read persistent storage, execute commands

    const off_t storeSize = lseek(*prevStore, 0, SEEK_END);
    if (storeSize == off_t(-1))
    {
      throw std::runtime_error(std::string("Failed to seek in persistent store: ") + strerror(errno));
    }

    void* pStore = mmap(nullptr, storeSize, PROT_READ, MAP_PRIVATE, *prevStore, 0);
    if (!pStore)
    {
      throw std::runtime_error(std::string("Failed to mmap persisten store: ") + strerror(errno));
    }

    // process pStore
    const char* pStoreBegin = reinterpret_cast<const char*>(pStore);
    ReadBuffer buffer(pStoreBegin, storeSize);
    std::size_t lastPos = 0;
    while (buffer && executeCommand(buffer))
    {
      lastPos = buffer.get() - pStoreBegin;
    }

    lseek(*prevStore, lastPos, SEEK_SET);

    munmap(pStore, storeSize);

    _persStore = std::move(prevStore);
  }
}

void Store::writePersStore(const iovec* pIovec, std::size_t vecSize, std::size_t fullSize)
{
  if (_persStore)
  {
    ssize_t wsize = writev(*_persStore, pIovec, vecSize);
    if (wsize < 0 || std::size_t(wsize) < fullSize)
    {
      throw std::runtime_error(std::string("Failed to write persistent store: ") + strerror(errno));
    }

    KVS_LOG_DEBUG << "Persistent storage write done, " << wsize << " bytes";
  } // else: persistent storage was turned off

//  else { KVS_LOG_DEBUG <<"Store turned off"; }
}

Store::Container::mapped_type& Store::operator[](const std::string& key)
{
  return _store[key];
}

Store::Container::iterator Store::find(const std::string& key)
{
  return _store.find(key);
}

Store::Container::const_iterator Store::find(const std::string& key) const
{
  return _store.find(key);
}

Store::Container::const_iterator Store::end() const
{
  return _store.end();
}

bool Store::executeCommand(ReadBuffer& reader)
{
  if (reader.size() < sizeof(command::Size) + sizeof(command::Tag))
  {
    KVS_LOG_WARNING << "Persistent store was too short";
    return false;
  }

  const char* comBegin = reader.get() + sizeof(command::Size);

  command::Size comSize = 0;
  reader.read(comSize);

  auto payloadSize = comSize - sizeof(comSize);

  if (reader.size() < payloadSize)
  {
    KVS_LOG_WARNING << "Chunk command found in persistent store";
    return false;
  }

  command::Tag comTag;
  reader.read(comTag);

  try
  {

    switch (comTag)
    {
    case command::Tag::SET:
    {
      SetCommand input(command::deserialize{}, comBegin, payloadSize);
      input.execute(*this);
      break;
    }
    case command::Tag::PUSH:
    {
      PushCommand input(command::deserialize{}, comBegin, payloadSize);
      input.execute(*this);
      break;
    }
    case command::Tag::POP:
    {
      PopCommand input(command::deserialize{}, comBegin, payloadSize);
      input.execute(*this);
      break;
    }
    default:
      KVS_LOG_WARNING << "Unknown command in persistent store: " << int(comTag);
      break;
    }

  }
  catch (const std::runtime_error& ex)
  {
    KVS_LOG_ERROR << "Failed to deserialize command while processing persistent store";
    return false;
  }

  reader.discard(payloadSize - sizeof(comTag));

  return true;
}

void Store::foreach(std::function<void(const std::string&, Container::mapped_type&)> func)
{
  for (auto&& pair : _store)
  {
    func(pair.first, pair.second);
  }
}

} // namespace kvs
