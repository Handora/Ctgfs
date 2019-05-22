// lock client interface.

#ifndef lock_client_h
#define lock_client_h

#include <string>
#include <vector>
#include "lock_protocol.h"
#include "rpc/rpc.h"

using namespace ::ctgfs::rpc;

namespace ctgfs {
namespace client {

// Client interface to the lock server
class lock_client {
 protected:
  rpcc* cl;

 public:
  lock_client(const std::string& d);
  void ConnectTo(const std::string&);
  virtual ~lock_client(){};
  virtual lock_protocol::status acquire(lock_protocol::lockid_t);
  virtual lock_protocol::status release(lock_protocol::lockid_t);
  virtual lock_protocol::status stat(lock_protocol::lockid_t);
};

}  // namespace client
}  // namespace ctgfs

#endif
