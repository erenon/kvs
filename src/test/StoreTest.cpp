
#include <kvs/Store.hpp>

#define BOOST_TEST_MODULE Store
#include <boost/test/unit_test.hpp>

using namespace kvs;

BOOST_AUTO_TEST_CASE(StoreBasics)
{
  Store store(nullptr);

  auto end = store.find(Key("foo"));
  BOOST_REQUIRE(end == store.end());

  auto&& entry = store[Key("bar")];
  entry.first = 123;

  auto entryIt = store.find(Key("bar"));
  BOOST_REQUIRE(entryIt->second.first == 123);
}
