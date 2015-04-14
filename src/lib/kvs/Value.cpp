#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

#include <kvs/Value.hpp>

namespace kvs {

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
  rule<std::vector<char>> str = qi::lexeme['"' >> +(qi::char_ - '"') >> '"'];

  auto scalar = float_ | double_ | uint | qi::int_ | str;

  auto afloat = '[' >> (float_ % ',') >> ']';
  auto adouble = '[' >> (double_ % ',') >> ']';
  auto auint = '[' >> (uint % ',') >> ']';
  auto aint = '[' >> (qi::int_ % ',') >> ']';

  return phrase_parse(
    begin,
    end,
    (scalar | afloat | adouble | auint | aint),
    space,
    result
  );
}

} // namespace kvs
