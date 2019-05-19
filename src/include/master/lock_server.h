// this is the lock server
// the lock client has a similar interface

#ifndef lock_server_h
#define lock_server_h

#include <string>
#include <map>
#include <master/lock_protocol.h>
#include "rpc/rpc.h"

namespace ctgfs {
namespace lock_server {

class lock_server {

 protected:
  int nacquire;

 public:
  lock_server();
  ~lock_server() {};
  lock_protocol::status stat(int clt, lock_protocol::lockid_t lid, int &);
  lock_protocol::status acquire(int clt, lock_protocol::lockid_t lid, int&);
  lock_protocol::status release(int clt, lock_protocol::lockid_t lid, int&);

 private:
  pthread_mutex_t lock_m_;
  std::map<lock_protocol::lockid_t, std::pair<bool, pthread_cond_t*> > lock_map_;
};

} // namespace lock_server
} // namespace ctgfs

#endif 







