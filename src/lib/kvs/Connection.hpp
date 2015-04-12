#ifndef KVS_CONNECTION_HPP_
#define KVS_CONNECTION_HPP_

#include <boost/utility/string_ref.hpp>

namespace kvs {

class Connection
{
public:
  typedef boost::string_ref Key;

  Connection();

  template <typename Field>
  bool get(const Key& key, Field& result);

  template <typename Field>
  void set(const Key& key, const Field& value);

  void unset(const Key& key);
};

} // namespace kvs

#endif // KVS_CONNECTION_HPP_
