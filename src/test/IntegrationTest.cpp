#include <thread>

#define BOOST_TEST_MODULE IntegrationTest
#include <boost/test/unit_test.hpp>

#include <boost/thread/latch.hpp>

#include <kvs/Reactor.hpp>
#include <kvs/ListenHandler.hpp>
#include <kvs/Store.hpp>
#include <kvs/Connection.hpp>

using namespace kvs;

void server(Reactor& reactor, int port, boost::latch& started)
{
  Store store("/tmp/kvs-inttest.db");
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
    server, std::ref(reactor), port, std::ref(serverStarted)
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
