#include <ostream>

#include <kvs/Value.hpp>

#define BOOST_TEST_MODULE Value
#include <boost/test/unit_test.hpp>

using namespace kvs;

namespace std {

template <typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& ts)
{
  out << typeid(T).name() << '[';
  for (auto&& t : ts)
  {
    out << t << ",";
  }
  out << ']';
  return out;
}

} // namespace std

template <typename T>
void checkMatch(const char* input, const T& expected)
{
  TypedValue result;
  BOOST_CHECK(readValue(input, input+strlen(input), result));
  BOOST_CHECK_EQUAL(result, TypedValue{expected});
}

BOOST_AUTO_TEST_CASE(ValueRead)
{
  checkMatch("0xFF", unsigned(255));
  checkMatch("123", unsigned(123));
  checkMatch("-123", int(-123));
  checkMatch("+123", int(+123));
  checkMatch("23.4f", 23.4f);
  checkMatch("23.4", 23.4);

  checkMatch("[0xFF, 0xFF]", std::vector<unsigned>{255, 255});
  checkMatch("[123, 256]", std::vector<unsigned>{123, 256});
  checkMatch("[-123, +256]", std::vector<int>{-123, 256});
  checkMatch("[+123, -256]", std::vector<int>{123, -256});
  checkMatch("[23.4f, 12.2f]", std::vector<float>{23.4f, 12.2f});
  checkMatch("[59.4, 55.6]", std::vector<double>{59.4, 55.6});

  checkMatch("\"foobar\"", std::vector<char>{'f', 'o', 'o', 'b', 'a', 'r'});
}
