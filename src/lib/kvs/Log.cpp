#include <boost/log/utility/setup/file.hpp>

#include <kvs/Log.hpp>

namespace kvs {

void openLogfile(const char* logfile)
{
  boost::log::add_file_log(logfile);
}

} // namespace kvs
