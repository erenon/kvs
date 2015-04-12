#include <cstdio>

#include <kvs/Log.hpp>
#include <kvs/Reactor.hpp>
#include <kvs/TextCommandHandler.hpp>
#include <kvs/ListenHandler.hpp>

using namespace kvs;

static bool g_runMainLoop = true;

int main(int argc, const char* argv[])
{
  (void)argc;
  (void)argv;

  Reactor reactor;

  // Add console
  reactor.addHandler<TextCommandHandler>(STDIN_FILENO, EPOLLIN, STDIN_FILENO);

  // Add server
  ListenHandler server(reactor, 1337);

  while (g_runMainLoop)
  {
    reactor.dispatch();
  }

  return 0;
}
