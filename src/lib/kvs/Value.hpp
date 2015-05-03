#ifndef KVS_VALUE_HPP_
#define KVS_VALUE_HPP_

#include <cstdint>
#include <cstring>
#include <vector>

#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#define BOOST_MPL_LIMIT_LIST_SIZE 30
#define BOOST_MPL_LIMIT_VECTOR_SIZE 30
#include <boost/variant/variant.hpp>
#include <boost/variant/get.hpp>

#include <kvs/Buffer.hpp>

namespace kvs {

enum ValueTag : uint16_t
{
  null   = 0,
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

struct NullValue
{
  static constexpr std::size_t serializedSize = sizeof(ValueTag::null);
  static const char* serializedValue();
};

inline constexpr bool operator==(const NullValue&, const NullValue&)
{ return true; }

std::ostream& operator<<(std::ostream& out, const NullValue&);

typedef boost::variant<
  NullValue,

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

  std::vector<NullValue>,

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

typedef std::size_t ListSize;

template <typename>
struct ValueDescriptor;

template <>
struct ValueDescriptor<NullValue>
{
  static constexpr ValueTag tag = ValueTag::null;

  static constexpr std::size_t size(const NullValue&)
  {
    return sizeof(ValueTag);
  }

  static void serialize(const NullValue&, char* buffer)
  {
    const ValueTag tag_ = tag;
    std::memcpy(buffer, &tag_, sizeof(tag_));
  }
};

template <typename T>
struct ValueDescriptor<std::vector<T>>
{
  static_assert(
    std::is_arithmetic<T>::value || std::is_same<T, NullValue>::value,
    "Unsupported list type"
  );

  static constexpr ValueTag tag =
    static_cast<ValueTag>(ValueDescriptor<T>::tag | ValueTag::list);

  static std::size_t size(const std::vector<T>& vec)
  {
    return sizeof(tag) + sizeof(ListSize) + vec.size() * sizeof(T);
  }

  static void serialize(const std::vector<T>& vec, char* buffer)
  {
    const ValueTag tag_ = tag;
    ListSize size = vec.size();

    FixWriteBuffer writer(buffer);
    writer.write(tag_);
    writer.write(size);
    writer.write(vec.data(), sizeof(T) * size);
  }
};

template <typename Arithmetic>
struct ValueDescriptor
{
  static_assert(std::is_arithmetic<Arithmetic>::value, "Unsupported type");

  static constexpr ValueTag tag =
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
  {
    return sizeof(tag) + sizeof(Arithmetic);
  }

  static void serialize(const Arithmetic& a, char* buffer)
  {
    const ValueTag tag_ = tag;
    FixWriteBuffer writer(buffer);
    writer.write(tag_);
    writer.write(a);
  }
};

namespace value {

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

std::size_t serializedSize(const TypedValue& value);

void serialize(const TypedValue& value, char* buffer);

TypedValue deserialize(const char* buffer, std::size_t bufferSize);

ValueTag deserializeTag(const char* buffer, std::size_t bufferSize);

} // namespace value

} // namespace kvs

#endif // KVS_VALUE_HPP_
