#ifndef KVS_VALUE_HPP_
#define KVS_VALUE_HPP_

#include <cstdint>
#include <cstring>
#include <vector>

#include <boost/variant/variant.hpp>
// #include <boost/utility/string_ref.hpp>

namespace kvs {

enum ValueType : uint16_t
{
  scalar = 0,
  list   = 1,

  int8_v,
  int16_v,
  int32_v,
  int64_v,

  uint8_v,
  uint16_v,
  uint32_v,
  uint64_v,

  float_v,
  double_v,
};

template <typename T>
struct ValueTypeOf
{
  static const ValueType value =
    (std::is_signed<T>::value)
  ? (sizeof(T) == 1)
  ? int8_v
  : (sizeof(T) == 2)
  ? int16_v
  : (sizeof(T) == 4)
  ? int32_v
  : int64_v
  : (sizeof(T) == 1)
  ? uint8_v
  : (sizeof(T) == 2)
  ? uint16_v
  : (sizeof(T) == 4)
  ? uint32_v
  : uint64_v;
};

template <>
struct ValueTypeOf<float>
{
  static const ValueType value = float_v;
};

template <>
struct ValueTypeOf<double>
{
  static const ValueType value = double_v;
};

template <typename T>
struct ValueTypeOf<std::vector<T>>
{
  static const ValueType value =
    ValueTypeOf<T>::value & ValueType::list;
};

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

//inline bool readValue(boost::string_ref& input, TypedValue& result)
//{
//  return readValue(&input.front(), &input.back() + 1, result);
//}

//class Value
//{
//public:
//  constexpr Value() = default;
//
//  template <typename T>
//  Value(const T& t)
//    :_type(ValueTypeOf<T>::value)
//  {
//    std::memcpy(
//      reinterpret_cast<void*>(this + 1),
//      reinterpret_cast<const void*>(&T),
//      sizeof(T)
//    );
//  }
//
//  template <typename T>
//  Value(const std::vector<T>& vect)
//    :_type(ValueTypeOf<std::vector<T>>::value)
//  {
//    // TODO Value vector constructor
//    static_assert(sizeof(T) == 0, "Not implemented");
//  }
//
//  constexpr ValueType type() const { return _type; }
//
//  template <typename T>
//  T& get()
//  {
//    return *reinterpret_cast<T*>(this + 1);
//  }
//
//  template <typename T>
//  const T& get() const
//  {
//    return *reinterpret_cast<const T*>(this + 1);
//  }
//
//  // TODO getVariant()
//  // TODO getOptional()
//  // TODO get vector specializations
//
//  uint16_t size() const
//  {
//    ValueType scalarType = _type & ValueType::scalar;
//    uint16_t scalarSize;
//
//    switch (scalarType)
//    {
//    case int8_v:
//    case uint8_v:
//      scalarSize = 1;
//      break;
//    case int16_v:
//    case uint16_v:
//      scalarSize = 2;
//      break;
//    case int32_v:
//    case uint32_v:
//      scalarSize = 4;
//      break;
//    case int64_v:
//    case uint64_v:
//      scalarSize = 8;
//      break;
//    case float_v:
//      scalarSize = sizeof(float);
//      break;
//    case double_v:
//      scalarSize = sizeof(double);
//      break;
//    default:
//      std::terminate();
//      break;
//    }
//
//    if (_type & ValueType::list)
//    {
//      uint16_t* pListSize = reinterpret_cast<const uint16_t*>(this + 1);
//      return sizeof(_type) + *pListSize * scalarSize;
//    }
//    else
//    {
//      return sizeof(_type) + scalarSize;
//    }
//  }
//
//private:
//  ValueType _type;
//  // payload follows
//} __attribute__((packed));

} // namespace kvs

#endif // KVS_VALUE_HPP_
