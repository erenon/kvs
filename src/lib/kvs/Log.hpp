#ifndef KVS_LOG_HPP_
#define KVS_LOG_HPP_

#include <boost/log/trivial.hpp>
#include <boost/log/utility/manipulators/dump.hpp>

namespace kvs {

#define KVS_LOG_DEBUG   BOOST_LOG_TRIVIAL(debug)
#define KVS_LOG_INFO    BOOST_LOG_TRIVIAL(info)
#define KVS_LOG_WARNING BOOST_LOG_TRIVIAL(warning)
#define KVS_LOG_ERROR   BOOST_LOG_TRIVIAL(error)

typedef boost::log::dump_manip LogArray;

void openLogfile(const char* logfile);

} // namespace kvs

#endif // KVS_LOG_HPP_
