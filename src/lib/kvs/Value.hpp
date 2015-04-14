#ifndef KVS_VALUE_HPP_
#define KVS_VALUE_HPP_

#include <cstdint>
#include <cstring>
#include <vector>

#include <boost/variant/variant.hpp>

namespace kvs {

typedef boost::variant<
  char,
  short,
  int,
  int64_t,

  unsigned char,
  unsigned short,
  unsigned int,
  uint64_t,

  float,
  double,

  std::vector<char>,
  std::vector<short>,
  std::vector<int>,
  std::vector<int64_t>,

  std::vector<unsigned char>,
  std::vector<unsigned short>,
  std::vector<unsigned int>,
  std::vector<uint64_t>,

  std::vector<float>,
  std::vector<double>
> TypedValue;

bool readValue(const char*& begin, const char* end, TypedValue& result);

enum ValueTag : uint16_t
{
  scalar = 0,
  list   = 1,

  tag_int8  = 2,
  tag_int16 = 4,
  tag_int32 = 6,
  tag_int64 = 8,

  tag_uint8  = 10,
  tag_uint16 = 12,
  tag_uint32 = 14,
  tag_uint64 = 16,

  tag_float  = 18,
  tag_double = 20,
};

typedef std::size_t ListSize;

template <typename>
struct ValueDescriptor;

template <typename T>
struct ValueDescriptor<std::vector<T>>
{
  static_assert(std::is_arithmetic<T>::value, "Unsupported list type");

  static const ValueTag tag = ValueDescriptor<T>::tag & ValueTag::list;

  static std::size_t size(const std::vector<T>& vec)
  {
    return sizeof(ListSize) + vec.size() * sizeof(T);
  }

  static void serialize(const std::vector<T>& vec, char* buffer)
  {
    ListSize size = vec.size();
    std::memcpy(buffer, &size, sizeof(size));
    std::memcpy(buffer + sizeof(size), vec.data(), sizeof(T) * size);
  }
};

template <typename Arithmetic>
struct ValueDescriptor
{
  static_assert(std::is_arithmetic<Arithmetic>::value, "Unsupported type");

  static const ValueTag tag =
    (std::is_same<Arithmetic, float>::value)
  ? tag_float
  : (std::is_same<Arithmetic, double>::value)
  ? tag_double
  : (std::is_signed<Arithmetic>::value)
  ? (sizeof(Arithmetic) == 1)
  ? tag_int8
  : (sizeof(Arithmetic) == 2)
  ? tag_int16
  : (sizeof(Arithmetic) == 4)
  ? tag_int32
  : tag_int64
  : (sizeof(Arithmetic) == 1)
  ? tag_uint8
  : (sizeof(Arithmetic) == 2)
  ? tag_uint16
  : (sizeof(Arithmetic) == 4)
  ? tag_uint32
  : tag_uint64;

  static constexpr std::size_t size(const Arithmetic&)
  { return sizeof(Arithmetic); }

  static void serialize(const Arithmetic& a, char* buffer)
  {
    std::memcpy(buffer, &a, sizeof(a));
  }
};

template <typename T>
std::size_t serializedSize(const T& t)
{
  return ValueDescriptor<T>::size(t);
}

template <typename T>
void serialize(const T& t, char* buffer)
{
  ValueDescriptor<T>::serialize(t, buffer);
}

} // namespace kvs

#endif // KVS_VALUE_HPP_
