#include <cstring>

#include <kvs/Store.hpp>

struct Backup
{
  Backup(const std::string& k, std::size_t s, std::unique_ptr<char[]>& v)
    :key("backup_" + k),
     size(s),
     value(new char[size])
  {
    std::memcpy(value.get(), v.get(), size);
  }

  std::string key;
  std::size_t size;
  std::unique_ptr<char[]> value;
};

extern "C" {

void kvs_procedure(kvs::Store& store)
{
  std::vector<Backup> backups;

  auto backup = [&](
    const std::string& key,
    std::pair<std::size_t, std::unique_ptr<char[]>>& value
  )
  {
    if (key.compare(0, 7, "backup_") != 0)
    {
      backups.emplace_back(key, value.first, value.second);
    }
  };

  store.foreach(backup);

  for (auto&& backup : backups)
  {
    auto& pair = store[backup.key];
    pair.first = backup.size;
    pair.second = std::move(backup.value);
  }
}

}
