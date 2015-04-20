#include <climits>
#include <iostream>
#include <cstring> // strerror

#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#define BOOST_MPL_LIMIT_LIST_SIZE 30
#define BOOST_MPL_LIMIT_VECTOR_SIZE 30
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

#include <kvs/TextCommandHandler.hpp>
#include <kvs/Reactor.hpp>
#include <kvs/Log.hpp>
#include <kvs/Command.hpp>
#include <kvs/Value.hpp>

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

TextCommandHandler::TextCommandHandler(
  int in,
  int out,
  Store& store,
  Reactor& reactor
)
  :_in(in),
   _out(out),
   _store(store),
   _reactor(reactor)
{}

bool TextCommandHandler::dispatch()
{
  char buff[LINE_MAX];
  ssize_t rsize = read(_in, buff, LINE_MAX);

  if (rsize > 0)
  {
    if (buff[rsize - 1] == '\n')
    {
      const char* start = &buff[0];
      start = processCommand(start, start + rsize - 1);
//      if (!start)
//      {
//
//      }
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

  return true;
}

namespace {

namespace qi = boost::spirit::qi;

struct TextCommands : qi::symbols<char, CommandType>
{
  TextCommands()
  {
    add
      ("get" , CommandType::GET)
      ("set" , CommandType::SET)
    ;
  }

} g_commands;

bool readCommand(const char*& begin, const char* end, CommandType& result)
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

bool readKey(const char*& begin, const char* end, Key& result)
{
  const char* space = std::find(begin, end, ' ');
  result = Key(begin, space - begin);
  begin = space;
  return true;
}

void writeCommand(const SetCommand& command, int fd)
{
  auto serializedValue = command.value();
  TypedValue value = value::deserialize(serializedValue.first, serializedValue.second);

  std::stringstream stream;
  stream << value << '\n';
  std::string textValue = stream.str();

  write(fd, textValue.data(), textValue.size());
}

} // namespace

const char* TextCommandHandler::processCommand(const char* buffer, const char* end)
{
  // get command type from buffer
  CommandType commandType;
  if (! readCommand(buffer, end, commandType))
  {
    std::string command(buffer, end);
    KVS_LOG_WARNING << "Unrecognized command: " << command;
    return nullptr;
  }

  switch (commandType)
  {
    case CommandType::GET:
    {
      Key key;
      if (! readKey(buffer, end, key)) { return nullptr; }

      GetCommand command(key);
      SetCommand result = command.execute(_store);

      writeCommand(result, _out);

      break;
    }
    case CommandType::SET:
    {
      Key key;
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
    default:
      return nullptr;
      break;
  }

  return buffer;
}

} // namespace
