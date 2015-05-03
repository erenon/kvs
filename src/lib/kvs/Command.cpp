#include <cstring> // memcpy

#include <kvs/Command.hpp>
#include <kvs/Store.hpp>
#include <kvs/Value.hpp>
#include <kvs/Buffer.hpp>
#include <kvs/Error.hpp>

namespace kvs {

const command::Tag GetCommand::_tag = command::Tag::GET;
const command::Tag SetCommand::_tag = command::Tag::SET;
const command::Tag AddCommand::_tag = command::Tag::ADD;
const command::Tag SumCommand::_tag = command::Tag::SUM;
const command::Tag MaxCommand::_tag = command::Tag::MAX;
const command::Tag MinCommand::_tag = command::Tag::MIN;

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
// ADD
//

AddCommand::AddCommand(command::deserialize, const char* buffer, command::Size size)
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

void AddCommand::execute(Store& store) const
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

std::pair<const char*, std::size_t> AddCommand::value() const
{
  return {_serializedValue, _serializedValueSize};
}

void AddCommand::serialize(iovec* output, command::Size& size) const
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

} // namespace
