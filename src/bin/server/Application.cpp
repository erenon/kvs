#include <cstdio>

#include <kvs/Log.hpp>
#include <kvs/Reactor.hpp>
#include <kvs/TextCommandHandler.hpp>
#include <kvs/ListenHandler.hpp>

using namespace kvs;

int main(int argc, const char* argv[])
{
  (void)argc;
  (void)argv;

  Reactor reactor;

  // Add console
  reactor.addHandler<TextCommandHandler>(
    STDIN_FILENO, EPOLLIN, STDIN_FILENO, reactor
  );

  // Add server
  ListenHandler server(reactor, 1337);

  while (! reactor.isStopped())
  {
    reactor.dispatch();
  }

  return 0;
}
