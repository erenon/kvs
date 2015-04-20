#include <cstdio>

#include <kvs/Log.hpp>
#include <kvs/Reactor.hpp>
#include <kvs/TextCommandHandler.hpp>
#include <kvs/ListenHandler.hpp>
#include <kvs/Store.hpp>

using namespace kvs;

int main(int argc, const char* argv[])
{
  (void)argc;
  (void)argv;

  Store store("/tmp/kvs_store.db");

  Reactor reactor;

  // Add console
  reactor.addHandler<TextCommandHandler>(
    STDIN_FILENO, EPOLLIN,
    STDIN_FILENO, STDOUT_FILENO, store, reactor
  );

  // Add server
  ListenHandler server(reactor, 1337);

  while (! reactor.isStopped())
  {
    reactor.dispatch();
  }

  return 0;
}
