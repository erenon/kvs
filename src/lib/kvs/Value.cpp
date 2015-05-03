#include <type_traits>

#include <kvs/Value.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

#include <kvs/Buffer.hpp>

namespace kvs {

namespace {
uint16_t g_serializedNullValue = ValueTag::null;
} // namespace

const char* NullValue::serializedValue()
{
  return reinterpret_cast<const char*>(&g_serializedNullValue);
}

std::ostream& operator<<(std::ostream& out, const NullValue&)
{
  out << "{null}";
  return out;
}

template <typename T>
using rule = boost::spirit::qi::rule<const char*, T(), boost::spirit::ascii::space_type>;

template <typename T>
using strict_real = boost::spirit::qi::real_parser<T, boost::spirit::qi::strict_real_policies<T>>;

bool readValue(const char*& begin, const char* end, TypedValue& result)
{
  namespace qi = boost::spirit::qi;
  namespace ascii = boost::spirit::ascii;

  using qi::phrase_parse;
  using ascii::space;

  rule<float> float_= qi::lexeme[strict_real<float>() >> 'f'];
  rule<double> double_= strict_real<double>();
  rule<unsigned> uint = qi::lexeme["0x" >> qi::hex] | qi::uint_;

  rule<std::vector<char>>     str = qi::lexeme['"' >> +(qi::char_ - '"') >> '"'];
  rule<std::vector<float>>    afloat  = '[' >> (float_ % ',') >> ']';
  rule<std::vector<double>>   adouble = '[' >> (double_ % ',') >> ']';
  rule<std::vector<unsigned>> auint   = '[' >> (uint % ',') >> ']';
  rule<std::vector<int>>      aint    = '[' >> (qi::int_ % ',') >> ']';

  return phrase_parse(
    begin,
    end,
    (float_ | double_ | uint | qi::int_ | str | afloat | adouble | auint | aint),
    space,
    result
  );
}

namespace value {

struct SerializedSizeVisitor : public boost::static_visitor<std::size_t>
{
  template <typename T>
  std::size_t operator()(const T& t) const { return serializedSize(t); }
};

std::size_t serializedSize(const TypedValue& value)
{
  SerializedSizeVisitor visitor;
  return boost::apply_visitor(visitor, value);
}

struct SerializeVisitor : public boost::static_visitor<>
{
  SerializeVisitor(char* buffer) :_buffer(buffer) {}
  char* _buffer;

  template <typename T>
  void operator()(const T& t) const { serialize(t, _buffer); }
};

void serialize(const TypedValue& value, char* buffer)
{
  SerializeVisitor visitor(buffer);

  boost::apply_visitor(visitor, value);
}

TypedValue deserialize(const char* buffer, std::size_t bufferSize)
{
  TypedValue result;
  ReadBuffer reader(buffer, bufferSize);

  std::underlying_type<ValueTag>::type tag;
  if (! reader.read(tag)) { return result; }

#define KVS_SCALAR_CASE(tag, type)   \
  case tag: \
  {         \
    type t; \
    if (! reader.read(t)) { break; } \
    result = t;                      \
    break;  \
  } \
  /**/

#define KVS_LIST_CASE(tag, type) \
  case tag: \
  { \
    std::vector<type> vt; \
    vt.reserve(listSize);\
    for (ListSize i = 0; i < listSize; ++i) \
    { \
      type t; \
      if (! reader.read(t)) { break; } \
      vt.push_back(t); \
    } \
    result = vt; \
    break; \
  } \
  /**/

  if (tag & ValueTag::list)
  {
    ListSize listSize;
    if (! reader.read(listSize)) { return result; }

    decltype(tag) scalarTag = tag & ~ValueTag::list;
    switch (scalarTag)
    {
      KVS_LIST_CASE(tag_int8, char)
      KVS_LIST_CASE(tag_int16, short)
      KVS_LIST_CASE(tag_int32, int)
      KVS_LIST_CASE(tag_int64, int64_t)
      KVS_LIST_CASE(tag_uint8, unsigned char)
      KVS_LIST_CASE(tag_uint16, unsigned short)
      KVS_LIST_CASE(tag_uint32, unsigned int)
      KVS_LIST_CASE(tag_uint64, uint64_t)
      KVS_LIST_CASE(tag_float, float)
      KVS_LIST_CASE(tag_double, double)
    }
  }
  else
  {
    switch (tag)
    {
      KVS_SCALAR_CASE(tag_int8, char)
      KVS_SCALAR_CASE(tag_int16, short)
      KVS_SCALAR_CASE(tag_int32, int)
      KVS_SCALAR_CASE(tag_int64, int64_t)
      KVS_SCALAR_CASE(tag_uint8, unsigned char)
      KVS_SCALAR_CASE(tag_uint16, unsigned short)
      KVS_SCALAR_CASE(tag_uint32, unsigned int)
      KVS_SCALAR_CASE(tag_uint64, uint64_t)
      KVS_SCALAR_CASE(tag_float, float)
      KVS_SCALAR_CASE(tag_double, double)
    }
  }

#undef KVS_SCALAR_CASE
#undef KVS_LIST_CASE

  return result;
}

ValueTag deserializeTag(const char* buffer, std::size_t bufferSize)
{
  ValueTag result = ValueTag::null;

  ReadBuffer reader(buffer, bufferSize);
  reader.read(result);

  return result;
}

} // namespace value

} // namespace kvs
