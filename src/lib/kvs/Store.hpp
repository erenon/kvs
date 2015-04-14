#ifndef KVS_STORE_HPP_
#define KVS_STORE_HPP_

#include <unordered_map>
#include <string>
#include <memory>

#include <kvs/Fd.hpp>

namespace kvs {

/**
 * TODO Store use boost::concurrent_unordered when ready or
 * TODO Store use boost::unordered_map and templated find/op[]
 */
class Store
{
  typedef std::unordered_map<std::string, std::unique_ptr<char[]>> Container;

public:
  Store(const char* persStore);

  void writePersStore(const char* command, const std::size_t size);

  std::unique_ptr<char[]>& operator[](const std::string& key);
  Container::iterator find(const std::string& key);
  Container::iterator end();

private:
  Fd _persStore;
  Container _store;
};

} // namespace kvs

#endif // KVS_STORE_HPP_
