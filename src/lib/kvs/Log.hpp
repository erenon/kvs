#ifndef KVS_LOG_HPP_
#define KVS_LOG_HPP_

#include <boost/log/trivial.hpp>
#include <boost/log/utility/manipulators/dump.hpp>
#include <boost/log/common.hpp>

namespace kvs {

boost::log::sources::severity_logger<boost::log::trivial::severity_level>&
getLogger();

#define KVS_LOG_DEBUG   BOOST_LOG_SEV(kvs::getLogger(), boost::log::trivial::debug)
#define KVS_LOG_INFO    BOOST_LOG_SEV(kvs::getLogger(), boost::log::trivial::info)
#define KVS_LOG_WARNING BOOST_LOG_SEV(kvs::getLogger(), boost::log::trivial::warning)
#define KVS_LOG_ERROR   BOOST_LOG_SEV(kvs::getLogger(), boost::log::trivial::error)

typedef boost::log::dump_manip LogArray;

void openLogfile(const char* logfile);

} // namespace kvs

#endif // KVS_LOG_HPP_
