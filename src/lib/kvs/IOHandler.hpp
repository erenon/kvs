#ifndef KVS_IOHANDLER_HPP_
#define KVS_IOHANDLER_HPP_

namespace kvs {

class IOHandler
{
public:
  virtual ~IOHandler() {}

  /** @returns true, if should reuse */
  virtual bool dispatch() = 0;
};

} // namespace kvs

#endif // KVS_IOHANDLER_HPP_
