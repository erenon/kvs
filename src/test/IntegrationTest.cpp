#include <thread>

#define BOOST_TEST_MODULE IntegrationTest
#include <boost/test/unit_test.hpp>

#include <boost/thread/latch.hpp>

#include <kvs/Reactor.hpp>
#include <kvs/ListenHandler.hpp>
#include <kvs/Store.hpp>
#include <kvs/Connection.hpp>

using namespace kvs;

void server(Reactor& reactor, int port, boost::latch& started, const char* pStore)
{
  Store store(pStore);
  ListenHandler server(reactor, port, store);

  started.count_down();

  while (! reactor.isStopped())
  {
    reactor.dispatch();
  }
}

BOOST_AUTO_TEST_CASE(ConnectSetGetInt)
{
  Reactor reactor;
  const int port = 1338;
  boost::latch serverStarted(1);

  std::thread serverThread(
    server, std::ref(reactor), port, std::ref(serverStarted), nullptr
  );

  serverStarted.wait();

  Connection connection("127.0.0.1", port);

  int val = 0;
  BOOST_CHECK(! connection.get("foo", val));
  connection.set<int>("foo", 123);
  BOOST_CHECK(connection.get("foo", val));
  BOOST_CHECK_EQUAL(123, val);

  reactor.stop();

  serverThread.join();

  BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(PersistentStore)
{
  unlink("/tmp/kvs-inttest.db");

  {
    Reactor reactor;
    const int port = 1339;
    boost::latch serverStarted(1);

    std::thread serverThread(
      server, std::ref(reactor), port, std::ref(serverStarted), "/tmp/kvs-inttest.db"
    );

    serverStarted.wait();

    Connection connection("127.0.0.1", port);

    connection.set<int>("foo", 123);
    connection.set<float>("bar", 456);
    connection.set<std::vector<int>>("baz", std::vector<int>{1, 2, 3});

    int foo = 0;
    BOOST_CHECK(connection.get("foo", foo));

    reactor.stop();

    serverThread.join();
  }

  {
    Reactor reactor;
    const int port = 1340;
    boost::latch serverStarted(1);

    std::thread serverThread(
      server, std::ref(reactor), port, std::ref(serverStarted), "/tmp/kvs-inttest.db"
    );

    serverStarted.wait();

    int foo = 0;
    float bar = 0;
    std::vector<int> baz;

    Connection connection("127.0.0.1", port);
    BOOST_CHECK(connection.get("foo", foo));
    BOOST_CHECK(connection.get("bar", bar));
    BOOST_CHECK(connection.get("baz", baz));

    BOOST_CHECK_EQUAL(foo, 123);
    BOOST_CHECK_EQUAL(bar, 456);
    BOOST_CHECK((baz == std::vector<int>{1, 2, 3}));

    reactor.stop();

    serverThread.join();
  }
}

BOOST_AUTO_TEST_CASE(AddCommandTest)
{
  Reactor reactor;
  const int port = 1338;
  boost::latch serverStarted(1);

  std::thread serverThread(
    server, std::ref(reactor), port, std::ref(serverStarted), nullptr
  );

  serverStarted.wait();

  Connection connection("127.0.0.1", port);

  connection.add("iarr", std::vector<int>{1,2,3});
  connection.add("iarr", int(4));
  connection.add("iarr", std::vector<int>{5,6,7});

  std::vector<int> iarr;
  BOOST_CHECK(connection.get("iarr", iarr));
  BOOST_CHECK((iarr == std::vector<int>{1,2,3,4,5,6,7}));

  reactor.stop();

  serverThread.join();

  BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(SumCommandTest)
{
  Reactor reactor;
  const int port = 1338;
  boost::latch serverStarted(1);

  std::thread serverThread(
    server, std::ref(reactor), port, std::ref(serverStarted), nullptr
  );

  serverStarted.wait();

  Connection connection("127.0.0.1", port);

  connection.add("iarr", std::vector<int>{1,2,3});
  connection.add("iarr", int(4));
  connection.add("iarr", std::vector<int>{5,6,7});

  int sum = 0;
  BOOST_CHECK(connection.sum("iarr", sum));
  BOOST_CHECK_EQUAL(28, sum);

  reactor.stop();

  serverThread.join();

  BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(MinMaxCommandTest)
{
  Reactor reactor;
  const int port = 1338;
  boost::latch serverStarted(1);

  std::thread serverThread(
    server, std::ref(reactor), port, std::ref(serverStarted), nullptr
  );

  serverStarted.wait();

  Connection connection("127.0.0.1", port);

  connection.set("iarr", std::vector<int>{-1, -2, -3, 0, 12, 5, 7, -9, 11, 6, 6, -1});

  int max = 0;
  int min = 0;

  BOOST_CHECK(connection.max("iarr", max));
  BOOST_CHECK(connection.min("iarr", min));

  BOOST_CHECK_EQUAL(12, max);
  BOOST_CHECK_EQUAL(-9, min);

  reactor.stop();

  serverThread.join();

  BOOST_CHECK(true);
}
