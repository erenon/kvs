#include <kvs/Config.hpp>
#include <kvs/Log.hpp>

#include <boost/property_tree/json_parser.hpp>

namespace kvs {

bool readConfig(Config& config, const std::string& path)
{
  try
  {
    boost::property_tree::read_json(path, config);
  }
  catch (const boost::property_tree::json_parser_error& ex)
  {
    KVS_LOG_ERROR << "Parser error while reading config file: '" << path << "':"
      << ex.what();
    return false;
  }

  return true;
}

} // namespace kvs
