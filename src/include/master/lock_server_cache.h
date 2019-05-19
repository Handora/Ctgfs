#ifndef lock_server_cache_h
#define lock_server_cache_h

#include <string>

#include <map>
#include <master/lock_protocol.h>
#include <rpc/rpc.h>
#include <master/lock_server.h>

namespace ctgfs {
namespace lock_server {

class lock_server_cache {
 private:
  int nacquire;
  std::map<lock_protocol::lockid_t, lock_serv_state> lsm_;
  pthread_mutex_t lock_m_;
 public:
  lock_server_cache();
  lock_protocol::status stat(lock_protocol::lockid_t, int &);
  int acquire(lock_protocol::lockid_t, std::string id, int &);
  int release(lock_protocol::lockid_t, std::string id, int &);
};

} // namespace lock
} // namespace ctgfs

#endif
