#ifndef KVS_STORE_HPP_
#define KVS_STORE_HPP_

#include <unordered_map>
#include <string>
#include <memory>

#include <sys/uio.h>

#include <kvs/Fd.hpp>
#include <kvs/Command.hpp>  // Key
#include <kvs/Buffer.hpp>

namespace kvs {

/**
 * TODO Store use boost::concurrent_unordered when ready or
 * TODO Store use boost::unordered_map and templated find/op[]
 */
class Store
{
  typedef std::unordered_map<
    std::string,
    std::pair<std::size_t, std::unique_ptr<char[]>>
  > Container;

public:
  Store(const char* persStore);

  void writePersStore(const iovec* pIovec, std::size_t vecSize, std::size_t fullSize);

  Container::mapped_type& operator[](const std::string& key);
  Container::iterator find(const std::string& key);
  Container::const_iterator find(const std::string& key) const;
  Container::const_iterator end() const;

  Container::mapped_type& operator[](const Key& key) { return (*this)[std::string(key)]; };
  Container::iterator find(const Key& key) { return find(std::string(key)); }
  Container::const_iterator find(const Key& key) const { return find(std::string(key)); }

private:
  bool executeCommand(ReadBuffer& buffer);

  Fd _persStore;
  Container _store;
};

} // namespace kvs

#endif // KVS_STORE_HPP_
