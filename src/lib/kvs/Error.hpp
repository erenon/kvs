#ifndef KVS_ERROR_HPP_
#define KVS_ERROR_HPP_

#include <cstring> // strerror, for clients

namespace kvs {

__attribute__ ((noreturn))
void failure(const char* msg);

void check(bool ok);

} // namespace kvs

#endif // KVS_ERROR_HPP_
