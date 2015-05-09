#include <boost/log/utility/setup/file.hpp>

#include <kvs/Log.hpp>

namespace kvs {

boost::log::sources::severity_logger<boost::log::trivial::severity_level>
g_logger;

boost::log::sources::severity_logger<boost::log::trivial::severity_level>&
getLogger()
{
  return g_logger;
}

void openLogfile(const char* logfile)
{
  boost::log::add_file_log(
    boost::log::keywords::file_name = logfile,
    boost::log::keywords::auto_flush = true
  );

  boost::log::core::get()->set_filter
  (
    boost::log::trivial::severity >= boost::log::trivial::trace
  );
}

} // namespace kvs
