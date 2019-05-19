// the lock server implementation

#include <arpa/inet.h>
#include <master/lock_server.h>
#include <stdio.h>
#include <unistd.h>
#include <sstream>

namespace ctgfs {
namespace lock_server {

lock_server::lock_server() : nacquire(0) {
  VERIFY(pthread_mutex_init(&lock_m_, 0) == 0);
}

lock_protocol::status lock_server::stat(int clt, lock_protocol::lockid_t lid,
                                        int& r) {
  lock_protocol::status ret = lock_protocol::OK;
  printf("stat request from clt %d\n", clt);
  r = nacquire;
  return ret;
}

lock_protocol::status lock_server::acquire(int clt, lock_protocol::lockid_t lid,
                                           int& r) {
  ScopedLock lm(&lock_m_);

  lock_protocol::status ret = lock_protocol::OK;
  while (true) {
    std::map<lock_protocol::lockid_t,
             std::pair<bool, pthread_cond_t*> >::iterator search =
        lock_map_.find(lid);
    if (search == lock_map_.end()) {
      pthread_cond_t* c = new pthread_cond_t();
      VERIFY(pthread_cond_init(c, 0) == 0);
      lock_map_[lid] = std::make_pair(true, c);
      return ret;
    }

    bool locked = search->second.first;
    pthread_cond_t* cond = search->second.second;
    if (locked == true) {
      pthread_cond_wait(cond, &lock_m_);
    } else {
      lock_map_[lid].first = true;
      return ret;
    }
  }
}

lock_protocol::status lock_server::release(int clt, lock_protocol::lockid_t lid,
                                           int& r) {
  ScopedLock lm(&lock_m_);

  lock_protocol::status ret = lock_protocol::OK;

  std::map<lock_protocol::lockid_t, std::pair<bool, pthread_cond_t*> >::iterator
      search = lock_map_.find(lid);
  if (search == lock_map_.end()) {
    return ret;
  }

  bool locked = search->second.first;
  pthread_cond_t* cond = search->second.second;
  if (locked == true) {
    search->second.first = false;
    pthread_cond_signal(cond);
  }
  return ret;
}

}  // lock_server
}  // ctgfs
