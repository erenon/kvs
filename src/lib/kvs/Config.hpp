#ifndef KVS_CONFIG_HPP_
#define KVS_CONFIG_HPP_

#include <string>

#include <boost/property_tree/ptree.hpp>

namespace kvs {

typedef boost::property_tree::ptree Config;

bool readConfig(Config& config, const std::string& path);

} // namespace kvs

#endif // KVS_CONFIG_HPP_
