#include <cstring> // memcpy
#include <map>

#include <kvs/Command.hpp>
#include <kvs/Store.hpp>
#include <kvs/Value.hpp>
#include <kvs/Buffer.hpp>
#include <kvs/Error.hpp>

#include <dlfcn.h>

namespace kvs {

const command::Tag GetCommand::_tag = command::Tag::GET;
const command::Tag SetCommand::_tag = command::Tag::SET;
const command::Tag PushCommand::_tag = command::Tag::PUSH;
const command::Tag PopCommand::_tag = command::Tag::POP;
const command::Tag SumCommand::_tag = command::Tag::SUM;
const command::Tag MaxCommand::_tag = command::Tag::MAX;
const command::Tag MinCommand::_tag = command::Tag::MIN;
const command::Tag SourceCommand::_tag = command::Tag::SOURCE;
const command::Tag ExecuteCommand::_tag = command::Tag::EXECUTE;

//
// SET
//

SetCommand::SetCommand(command::deserialize, const char* buffer, command::Size size)
{
  ReadBuffer reader(buffer, size);

  command::Tag actualTag;

  check(reader.read(actualTag));
  check(actualTag == _tag);
  check(reader.read(_key));
  check(reader.read(_serializedValueSize));
  _serializedValue = reader.get();
}

void SetCommand::execute(Store& store) const
{
  // write persistent store
  {
    iovec serialized[serializedVectorSize];
    std::size_t fullSize;
    serialize(serialized, fullSize);
    store.writePersStore(serialized, serializedVectorSize, fullSize);
  }

  auto&& entry = store[_key];
  if (entry.first < _serializedValueSize)
  {
    entry.second.reset(new char[_serializedValueSize]);
  }

  memcpy(entry.second.get(), _serializedValue, _serializedValueSize);
  entry.first = _serializedValueSize;
}

std::pair<const char*, std::size_t> SetCommand::value() const
{
  return {_serializedValue, _serializedValueSize};
}

void SetCommand::serialize(iovec* output, command::Size& size) const
{
  size = sizeof(size) + sizeof(_tag) + _key.size() + 1 + sizeof(_serializedValueSize) + _serializedValueSize;

  output[0].iov_base = &size;
  output[0].iov_len = sizeof(size);

  output[1].iov_base = const_cast<command::Tag*>(&_tag);
  output[1].iov_len = sizeof(_tag);

  output[2].iov_base = const_cast<char*>(_key.data());
  output[2].iov_len = _key.size() + 1;

  output[3].iov_base = const_cast<std::size_t*>(&_serializedValueSize);
  output[3].iov_len = sizeof(_serializedValueSize);

  output[4].iov_base = const_cast<char*>(_serializedValue);
  output[4].iov_len = _serializedValueSize;
}

//
// GET
//

GetCommand::GetCommand(command::deserialize, const char* buffer, command::Size size)
{
  ReadBuffer reader(buffer, size);
  command::Tag actualTag;

  check(reader.read(actualTag));
  check(actualTag == _tag);
  check(reader.read(_key));
}

SetCommand GetCommand::execute(const Store& store) const
{
  auto finder = store.find(_key);
  if (finder != store.end())
  {
    // *finder is the requested value
    SetCommand result(_key, finder->second.first, finder->second.second.get());
    return result;
  }
  else
  {
    // not found
    SetCommand result(_key, NullValue::serializedSize, NullValue::serializedValue());
    return result;
  }
}

void GetCommand::serialize(iovec* output, command::Size& size) const
{
  size = sizeof(size) + sizeof(_tag) + _key.size() + 1;

  output[0].iov_base = &size;
  output[0].iov_len = sizeof(size);

  output[1].iov_base = const_cast<command::Tag*>(&_tag);
  output[1].iov_len = sizeof(_tag);

  output[2].iov_base = const_cast<char*>(_key.data());
  output[2].iov_len = _key.size() + 1;
}

//
// PUSH
//

PushCommand::PushCommand(command::deserialize, const char* buffer, command::Size size)
{
  ReadBuffer reader(buffer, size);

  command::Tag actualTag;

  check(reader.read(actualTag));
  check(actualTag == _tag);
  check(reader.read(_key));
  check(reader.read(_serializedValueSize));
  _serializedValue = reader.get();
}

struct CreateSingularList : public boost::static_visitor<TypedValue>
{
  TypedValue operator()(const NullValue&) const
  {
    TypedValue result = NullValue{};
    return result;
  }

  template <typename T>
  TypedValue operator()(const T& item) const
  {
    std::vector<T> list{item};
    TypedValue result(list);
    return result;
  }

  template <typename T>
  TypedValue operator()(const std::vector<T>& items) const
  {
    TypedValue result(items);
    return result;
  }
};

struct PushBackIfSame : public boost::static_visitor<>
{
  // TODO PushBackIfSame use is convertible to T
  template <typename T>
  void operator()(std::vector<T>& list, const T& item) const
  {
    list.push_back(item);
  }

  template <typename T>
  void operator()(std::vector<T>& list, const std::vector<T>& items) const
  {
    list.insert(list.end(), items.begin(), items.end());
  }

  template <typename T, typename U>
  void operator()(const T&, const U&) const {}
};

void PushCommand::execute(Store& store) const
{
  // write persistent store
  {
    iovec serialized[serializedVectorSize];
    std::size_t fullSize;
    serialize(serialized, fullSize);
    store.writePersStore(serialized, serializedVectorSize, fullSize);
  }

  // get field
  auto&& entry = store[_key];

  TypedValue result;

  // if has content
  if (entry.first > 0)
  {
    // if type match
    ValueTag actualTag = value::deserializeTag(entry.second.get(), entry.first);
    ValueTag expectedTag = static_cast<ValueTag>(
      ValueTag::list | value::deserializeTag(_serializedValue, _serializedValueSize)
    );
    if (actualTag == expectedTag)
    {
      // deserialize
      result = value::deserialize(entry.second.get(), entry.first);
      TypedValue item = value::deserialize(_serializedValue, _serializedValueSize);
      // add item
      boost::apply_visitor(PushBackIfSame{}, result, item);
    }
    // else error
    else
    {
      // TODO send back error?
      KVS_LOG_WARNING << "Add command: type mismatch"
        " (exp: " << int(expectedTag) << ", act: " << int(actualTag) << ")";
      return;
    }
  }
  // else no content
  else
  {
    // create serialized singular list
    TypedValue value = value::deserialize(_serializedValue, _serializedValueSize);
    result = boost::apply_visitor(CreateSingularList{}, value);
  }

  // set content
  std::size_t newSize = value::serializedSize(result);
  if (entry.first < newSize)
  {
    entry.second.reset(new char[newSize]);
  }
  value::serialize(result, entry.second.get());
  entry.first = newSize;
}

std::pair<const char*, std::size_t> PushCommand::value() const
{
  return {_serializedValue, _serializedValueSize};
}

void PushCommand::serialize(iovec* output, command::Size& size) const
{
  size = sizeof(size) + sizeof(_tag) + _key.size() + 1 + sizeof(_serializedValueSize) + _serializedValueSize;

  output[0].iov_base = &size;
  output[0].iov_len = sizeof(size);

  output[1].iov_base = const_cast<command::Tag*>(&_tag);
  output[1].iov_len = sizeof(_tag);

  output[2].iov_base = const_cast<char*>(_key.data());
  output[2].iov_len = _key.size() + 1;

  output[3].iov_base = const_cast<std::size_t*>(&_serializedValueSize);
  output[3].iov_len = sizeof(_serializedValueSize);

  output[4].iov_base = const_cast<char*>(_serializedValue);
  output[4].iov_len = _serializedValueSize;
}

//
// POP
//

struct PopBack : public boost::static_visitor<>
{
  template <typename T>
  void operator()(std::vector<T>& list) const
  {
    list.pop_back();
  }

  template <typename T>
  void operator()(const T&) const {}
};

PopCommand::PopCommand(command::deserialize, const char* buffer, command::Size size)
{
  ReadBuffer reader(buffer, size);
  command::Tag actualTag;

  check(reader.read(actualTag));
  check(actualTag == _tag);
  check(reader.read(_key));
}

void PopCommand::execute(Store& store) const
{
  // write persistent store
  {
    iovec serialized[serializedVectorSize];
    std::size_t fullSize;
    serialize(serialized, fullSize);
    store.writePersStore(serialized, serializedVectorSize, fullSize);
  }

  // get field
  auto&& entry = store[_key];

  // if has content
  if (entry.first > 0)
  {
    // deserialize
    TypedValue list = value::deserialize(entry.second.get(), entry.first);
    // add item
    boost::apply_visitor(PopBack{}, list);

    // set content
    std::size_t newSize = value::serializedSize(list);
    if (entry.first < newSize)
    {
      entry.second.reset(new char[newSize]);
    }
    value::serialize(list, entry.second.get());
    entry.first = newSize;
  }
  // else no content, nop
}

void PopCommand::serialize(iovec* output, command::Size& size) const
{
  size = sizeof(size) + sizeof(_tag) + _key.size() + 1;

  output[0].iov_base = &size;
  output[0].iov_len = sizeof(size);

  output[1].iov_base = const_cast<command::Tag*>(&_tag);
  output[1].iov_len = sizeof(_tag);

  output[2].iov_base = const_cast<char*>(_key.data());
  output[2].iov_len = _key.size() + 1;
}

//
// SUM
//

struct SumList : public boost::static_visitor<TypedValue>
{
  template <typename T>
  TypedValue operator()(const T&) const
  {
    TypedValue result = NullValue{};
    return result;
  }

  template <typename T>
  TypedValue operator()(const std::vector<T>& items) const
  {
    T sum = 0;
    for (auto&& item : items)
    {
      sum += item;
    }
    TypedValue result(sum);
    return result;
  }
};

SumCommand::SumCommand(command::deserialize, const char* buffer, command::Size size)
{
  ReadBuffer reader(buffer, size);
  command::Tag actualTag;

  check(reader.read(actualTag));
  check(actualTag == _tag);
  check(reader.read(_key));
}

SetCommand SumCommand::execute(const Store& store) const
{
  auto finder = store.find(_key);
  if (finder != store.end())
  {
    // *finder is the requested value
    TypedValue maybeList = value::deserialize(finder->second.second.get(), finder->second.first);
    TypedValue sumVal = boost::apply_visitor(SumList{}, maybeList);

    char buffer[64];
    std::size_t serSize = value::serializedSize(sumVal);

    if (serSize <= 64)
    {
      value::serialize(sumVal, buffer);
      SetCommand result(_key, serSize, buffer);
      return result;
    }
    else
    {
      KVS_LOG_ERROR << "Sum result serialized size was too large: " << serSize;
    }
  }

  // not found (or too large)
  SetCommand result(_key, NullValue::serializedSize, NullValue::serializedValue());
  return result;
}

void SumCommand::serialize(iovec* output, command::Size& size) const
{
  size = sizeof(size) + sizeof(_tag) + _key.size() + 1;

  output[0].iov_base = &size;
  output[0].iov_len = sizeof(size);

  output[1].iov_base = const_cast<command::Tag*>(&_tag);
  output[1].iov_len = sizeof(_tag);

  output[2].iov_base = const_cast<char*>(_key.data());
  output[2].iov_len = _key.size() + 1;
}

//
// MAX
//

struct GetMax : public boost::static_visitor<TypedValue>
{
  template <typename T>
  TypedValue operator()(const T&) const
  {
    TypedValue result = NullValue{};
    return result;
  }

  template <typename T>
  TypedValue operator()(const std::vector<T>& items) const
  {
    T max = *std::max_element(items.begin(), items.end());
    TypedValue result = max;
    return result;
  }
};

MaxCommand::MaxCommand(command::deserialize, const char* buffer, command::Size size)
{
  ReadBuffer reader(buffer, size);
  command::Tag actualTag;

  check(reader.read(actualTag));
  check(actualTag == _tag);
  check(reader.read(_key));
}

SetCommand MaxCommand::execute(const Store& store) const
{
  auto finder = store.find(_key);
  if (finder != store.end())
  {
    // *finder is the requested value
    TypedValue maybeList = value::deserialize(finder->second.second.get(), finder->second.first);
    TypedValue maxVal = boost::apply_visitor(GetMax{}, maybeList);

    char buffer[64];
    std::size_t serSize = value::serializedSize(maxVal);

    if (serSize <= 64)
    {
      value::serialize(maxVal, buffer);
      SetCommand result(_key, serSize, buffer);
      return result;
    }
    else
    {
      KVS_LOG_ERROR << "Max result serialized size was too large: " << serSize;
    }
  }

  // not found (or too large)
  SetCommand result(_key, NullValue::serializedSize, NullValue::serializedValue());
  return result;
}

void MaxCommand::serialize(iovec* output, command::Size& size) const
{
  size = sizeof(size) + sizeof(_tag) + _key.size() + 1;

  output[0].iov_base = &size;
  output[0].iov_len = sizeof(size);

  output[1].iov_base = const_cast<command::Tag*>(&_tag);
  output[1].iov_len = sizeof(_tag);

  output[2].iov_base = const_cast<char*>(_key.data());
  output[2].iov_len = _key.size() + 1;
}

//
// MIN
//

struct GetMin : public boost::static_visitor<TypedValue>
{
  template <typename T>
  TypedValue operator()(const T&) const
  {
    TypedValue result = NullValue{};
    return result;
  }

  template <typename T>
  TypedValue operator()(const std::vector<T>& items) const
  {
    T min = *std::min_element(items.begin(), items.end());
    TypedValue result = min;
    return result;
  }
};

MinCommand::MinCommand(command::deserialize, const char* buffer, command::Size size)
{
  ReadBuffer reader(buffer, size);
  command::Tag actualTag;

  check(reader.read(actualTag));
  check(actualTag == _tag);
  check(reader.read(_key));
}

SetCommand MinCommand::execute(const Store& store) const
{
  auto finder = store.find(_key);
  if (finder != store.end())
  {
    // *finder is the requested value
    TypedValue maybeList = value::deserialize(finder->second.second.get(), finder->second.first);
    TypedValue maxVal = boost::apply_visitor(GetMin{}, maybeList);

    char buffer[64];
    std::size_t serSize = value::serializedSize(maxVal);

    if (serSize <= 64)
    {
      value::serialize(maxVal, buffer);
      SetCommand result(_key, serSize, buffer);
      return result;
    }
    else
    {
      KVS_LOG_ERROR << "Max result serialized size was too large: " << serSize;
    }
  }

  // not found (or too large)
  SetCommand result(_key, NullValue::serializedSize, NullValue::serializedValue());
  return result;
}

void MinCommand::serialize(iovec* output, command::Size& size) const
{
  size = sizeof(size) + sizeof(_tag) + _key.size() + 1;

  output[0].iov_base = &size;
  output[0].iov_len = sizeof(size);

  output[1].iov_base = const_cast<command::Tag*>(&_tag);
  output[1].iov_len = sizeof(_tag);

  output[2].iov_base = const_cast<char*>(_key.data());
  output[2].iov_len = _key.size() + 1;
}

//
// SOURCE
//

std::map<std::string, void(*)(Store&)> g_storedProcedures;

SourceCommand::SourceCommand(command::deserialize, const char* buffer, command::Size size)
{
  ReadBuffer reader(buffer, size);
  command::Tag actualTag;

  check(reader.read(actualTag));
  check(actualTag == _tag);
  check(reader.read(_key));
}

void SourceCommand::execute()
{
  void* lib = dlopen(_key.data(), RTLD_LAZY);

  if (!lib)
  {
    KVS_LOG_WARNING << "Failed to load library: '" << _key << "', error: "
      << dlerror();
    return;
  }

  void* procedure = dlsym(lib, "kvs_procedure");

  if (!procedure)
  {
    KVS_LOG_WARNING << "No kvs_procedure found in library: '" << _key << "', error: "
      << dlerror();
    return;
  }

  Key name = _key;

  { // beautify name
    auto slash = name.find_last_of('/');
    if (slash != Key::npos) { name.remove_prefix(slash); }
    if (name.starts_with("/lib")) { name.remove_prefix(4); }
    if (name.ends_with(".so")) { name.remove_suffix(3); }
  }

  g_storedProcedures.emplace(name.to_string(), reinterpret_cast<void(*)(Store&)>(procedure));

  KVS_LOG_INFO << "Procedure loaded: " << name;
}

void SourceCommand::serialize(iovec* output, command::Size& size) const
{
  size = sizeof(size) + sizeof(_tag) + _key.size() + 1;

  output[0].iov_base = &size;
  output[0].iov_len = sizeof(size);

  output[1].iov_base = const_cast<command::Tag*>(&_tag);
  output[1].iov_len = sizeof(_tag);

  output[2].iov_base = const_cast<char*>(_key.data());
  output[2].iov_len = _key.size() + 1;
}

//
// EXECUTE
//

ExecuteCommand::ExecuteCommand(command::deserialize, const char* buffer, command::Size size)
{
  ReadBuffer reader(buffer, size);
  command::Tag actualTag;

  check(reader.read(actualTag));
  check(actualTag == _tag);
  check(reader.read(_key));
}

void ExecuteCommand::execute(Store& store)
{
  auto finder = g_storedProcedures.find(std::string(_key));
  if (finder == g_storedProcedures.end())
  {
    KVS_LOG_WARNING << "Procedure not found: '" << _key << "'";
    return;
  }

  finder->second(store);

  KVS_LOG_INFO << "Procedure executed: " << _key;

  // TODO snapshot store
}

void ExecuteCommand::serialize(iovec* output, command::Size& size) const
{
  size = sizeof(size) + sizeof(_tag) + _key.size() + 1;

  output[0].iov_base = &size;
  output[0].iov_len = sizeof(size);

  output[1].iov_base = const_cast<command::Tag*>(&_tag);
  output[1].iov_len = sizeof(_tag);

  output[2].iov_base = const_cast<char*>(_key.data());
  output[2].iov_len = _key.size() + 1;
}

} // namespace
