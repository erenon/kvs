#include <climits>
#include <iostream>
#include <cstring> // strerror

#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#define BOOST_MPL_LIMIT_LIST_SIZE 30
#define BOOST_MPL_LIMIT_VECTOR_SIZE 30
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

#include <kvs/ConsoleCommandHandler.hpp>
#include <kvs/Reactor.hpp>
#include <kvs/Log.hpp>
#include <kvs/Command.hpp>
#include <kvs/Value.hpp>
#include <kvs/Error.hpp>

namespace std {

// Required to be able to ostream TypedValue

template <typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& ts)
{
  out << '[';

  bool first = true;
  for (auto&& t : ts)
  {
    if (first) { first = false; }
    else { out << ", "; }

    out << t;
  }

  out << ']';
  return out;
}

} // namespace std

namespace kvs {

ConsoleCommandHandler::ConsoleCommandHandler(
  int in,
  int out,
  Store& store,
  Reactor& reactor
)
  :_in(in),
   _out(out),
   _store(store),
   _reactor(reactor)
{
  ssize_t wsize = write(_out, "> ", 2);
  (void)wsize;
}

ConsoleCommandHandler::~ConsoleCommandHandler()
{
  if (_in == STDIN_FILENO)
  {
    fclose(stdin);
  }
  close(_in);
}

bool ConsoleCommandHandler::dispatch()
{
  char buff[LINE_MAX];
  ssize_t rsize = read(_in, buff, LINE_MAX);

  if (rsize > 0)
  {
    if (buff[rsize - 1] == '\n')
    {
      const char* start = &buff[0];
      start = processCommand(start, start + rsize - 1);
      if (!start)
      {
        KVS_LOG_WARNING << "Failed to process command";
      }
      // TODO process subsequent commands
    }
    else
    {
      KVS_LOG_WARNING << "TextCommandHandler: input too long";
      // consume all
      while (rsize == LINE_MAX)
      {
        rsize = read(_in, buff, LINE_MAX);
      }
    }
  }
  else if (rsize == 0)
  {
    _reactor.stop();
    return false;
  }
  else
  {
    KVS_LOG_ERROR << "TextCommandHandler read error: " << strerror(errno);
    return false;
  }

  ssize_t wsize = write(_out, "> ", 2);
  (void)wsize;

  return true;
}

namespace {

namespace qi = boost::spirit::qi;

struct TextCommands : qi::symbols<char, command::Tag>
{
  TextCommands()
  {
    add
      ("get" , command::Tag::GET)
      ("set" , command::Tag::SET)
      ("push" , command::Tag::PUSH)
      ("pop" , command::Tag::POP)
      ("sum" , command::Tag::SUM)
      ("max" , command::Tag::MAX)
      ("min" , command::Tag::MIN)
      ("source" , command::Tag::SOURCE)
      ("execute" , command::Tag::EXECUTE)
    ;
  }

} g_commands;

bool readCommand(const char*& begin, const char* end, command::Tag& result)
{
  using boost::spirit::ascii::space;

  return qi::phrase_parse(
    begin,
    end,
    qi::no_case[g_commands],
    space,
    result
  );
}

bool readKey(const char*& begin, const char* end, std::string& result)
{
  while (begin != end && *begin == ' ') { ++begin; }
  const char* space = std::find(begin, end, ' ');
  result.assign(begin, space);
  begin = space;
  return result.size();
}

void writeCommand(const SetCommand& command, int fd)
{
  auto serializedValue = command.value();
  TypedValue value = value::deserialize(serializedValue.first, serializedValue.second);

  std::stringstream stream;
  stream << value << '\n';
  std::string textValue = stream.str();

  std::size_t wsize = write(fd, textValue.data(), textValue.size());
  if (wsize != textValue.size()) { failure("ConsoleCommandHandler write");}
}

} // namespace

const char* ConsoleCommandHandler::processCommand(const char* buffer, const char* end)
{
  // get command type from buffer
  command::Tag commandTag;
  if (! readCommand(buffer, end, commandTag))
  {
    std::string command(buffer, end);
    KVS_LOG_WARNING << "Unrecognized command: " << command;
    return nullptr;
  }

  switch (commandTag)
  {
    case command::Tag::GET:
    {
      std::string key;
      if (! readKey(buffer, end, key)) { return nullptr; }

      GetCommand command(key);
      SetCommand result = command.execute(_store);

      writeCommand(result, _out);

      break;
    }
    case command::Tag::SET:
    {
      std::string key;
      if (! readKey(buffer, end, key)) { return nullptr; }

      TypedValue value;
      if (! readValue(buffer, end, value)) { return nullptr; }

      const std::size_t valueSize = value::serializedSize(value);
      std::unique_ptr<char[]> valueBuffer(new char[valueSize]);
      value::serialize(value, valueBuffer.get());

      KVS_LOG_DEBUG << "Set value: " << LogArray(valueBuffer.get(), valueSize);

      SetCommand command(key, valueSize, valueBuffer.get());
      command.execute(_store);

      break;
    }
    case command::Tag::PUSH:
    {
      std::string key;
      if (! readKey(buffer, end, key)) { return nullptr; }

      TypedValue value;
      if (! readValue(buffer, end, value)) { return nullptr; }

      const std::size_t valueSize = value::serializedSize(value);
      std::unique_ptr<char[]> valueBuffer(new char[valueSize]);
      value::serialize(value, valueBuffer.get());

      KVS_LOG_DEBUG << "Push value: " << LogArray(valueBuffer.get(), valueSize);

      PushCommand command(key, valueSize, valueBuffer.get());
      command.execute(_store);

      break;
    }
    case command::Tag::POP:
    {
      std::string key;
      if (! readKey(buffer, end, key)) { return nullptr; }

      PopCommand command(key);
      command.execute(_store);

      break;
    }
    case command::Tag::SUM:
    {
      std::string key;
      if (! readKey(buffer, end, key)) { return nullptr; }

      SumCommand command(key);
      SetCommand result = command.execute(_store);

      writeCommand(result, _out);

      break;
    }
    case command::Tag::MAX:
    {
      std::string key;
      if (! readKey(buffer, end, key)) { return nullptr; }

      MaxCommand command(key);
      SetCommand result = command.execute(_store);

      writeCommand(result, _out);

      break;
    }
    case command::Tag::MIN:
    {
      std::string key;
      if (! readKey(buffer, end, key)) { return nullptr; }

      MinCommand command(key);
      SetCommand result = command.execute(_store);

      writeCommand(result, _out);

      break;
    }
    case command::Tag::SOURCE:
    {
      std::string key;
      if (! readKey(buffer, end, key)) { return nullptr; }

      SourceCommand command(key);
      command.execute();

      break;
    }
    case command::Tag::EXECUTE:
    {
      std::string key;
      if (! readKey(buffer, end, key)) { return nullptr; }

      ExecuteCommand command(key);
      command.execute(_store);

      break;
    }
    default:
      return nullptr;
      break;
  }

  return buffer;
}

} // namespace
