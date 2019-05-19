// the caching lock server implementation

#include <master/lock_server_cache.h>
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "rpc/verify.h"
#include <master/handle.h>
#include <util/util.h>

namespace ctgfs {
namespace lock_server {

lock_server_cache::lock_server_cache()
{
  VERIFY(pthread_mutex_init(&lock_m_, 0) == 0);
}


int 
lock_server_cache::acquire(lock_protocol::lockid_t lid, std::string id, 
                               int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
  bool need_revoke = false;
  r = lock_protocol::OK;

  pthread_mutex_lock(&lock_m_);
  CTG_INFO("Server get acquire message from %s with lock %llu\n", id.c_str(), lid);

  if (lsm_.find(lid) == lsm_.end()) {
    lsm_[lid] = lock_serv_state();
  }

  if (lsm_[lid].holded) {
    if (lsm_[lid].holder != id) {
      // 队列里面无人，此时需要发revoke消息拿回锁
      if (lsm_[lid].pendingers.size() == 0) {
        lsm_[lid].pendingers.push_back(id);
        need_revoke = true;
        CTG_WARN("Server send revoke message to %s with lock %llu, Need RETRY!!!\n", lsm_[lid].holder.c_str(), lid);
        
        r = lock_protocol::RETRY;
      } else {
        // 队列还有等待，返回RETRY
        lsm_[lid].pendingers.push_back(id);
        CTG_WARN("Server fail to give to %s with lock %llu, Need RETRY!!!\n", id.c_str(), lid);

        r = lock_protocol::RETRY;
      } 
    } else {
      // 锁被自己锁住
      CTG_INFO("Server give to %s with lock %llu\n", id.c_str(), lid);
      lsm_[lid].holded = true;
      lsm_[lid].holder = id;

      if (lsm_[lid].pendingers.size()) {
        // 如果还有很多pendingers，需要做完之后放锁
        r = lock_protocol::RETIRE;
      }
    }
  } else {
    if (lsm_[lid].pendingers.size()) {
      CTG_INFO("Why there is not holding while having penderings\n");
    } else {
      // 没有人持锁，可以拿到锁
      CTG_INFO("Server give to %s with lock %llu\n", id.c_str(), lid);
      lsm_[lid].holded = true;
      lsm_[lid].holder = id;
    }
  }

  if (need_revoke) {
    std::string holder = lsm_[lid].holder;
    handle h(lsm_[lid].holder);
    rpcc *cl = h.safebind();
    pthread_mutex_unlock(&lock_m_);
    if (!cl) {
      return lock_protocol::IOERR;
    }
    int r2 = 0;
    ret = cl->call(rlock_protocol::revoke, lid, r2);
    if (ret != rlock_protocol::OK) {
      return lock_protocol::RPCERR;
    }

    
    CTG_INFO("Server get revoke rpc back from %s with lock %llu\n", holder.c_str(), lid);
    pthread_mutex_lock(&lock_m_);
  }

  pthread_mutex_unlock(&lock_m_);
  return ret;
}

int 
lock_server_cache::release(lock_protocol::lockid_t lid, std::string id, 
         int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
  r = rlock_protocol::OK;

  pthread_mutex_lock(&lock_m_);
  CTG_INFO("Server get release message %s with lock %llu\n", id.c_str(), lid);

  if (lsm_.find(lid) == lsm_.end()) {
    lsm_[lid] = lock_serv_state();
  }

  if (lsm_[lid].holded) {
    if (lsm_[lid].holder == id) {
      if (lsm_[lid].pendingers.size() != 0) {
        // 有等待，返回让其retry
        std::string next = lsm_[lid].pendingers.front();
        lsm_[lid].pendingers.erase(lsm_[lid].pendingers.begin());
        CTG_INFO("Server release lock %llu, and will let %s retry\n", lid, next.c_str());
        lsm_[lid].holded = true;
        lsm_[lid].holder = next;
        handle h(next);
        rpcc *cl = h.safebind();
        pthread_mutex_unlock(&lock_m_);
        if (!cl) {
          return lock_protocol::IOERR;
        }
        int r;
        ret = cl->call(rlock_protocol::retry, lid, r);
        if (ret != rlock_protocol::OK) {
          return lock_protocol::RPCERR;
        }
        CTG_INFO("Server get retry rpc back from %s with lock %llu\n", next.c_str(), lid);
        pthread_mutex_lock(&lock_m_);
      } else {
        // 没有正在等待，返回FREE状态
        CTG_INFO("Server release lock %llu, and will be in FREE state\n", lid);
        lsm_[lid].holded = false;
        lsm_[lid].holder = "";
      }
    }
  }

  pthread_mutex_unlock(&lock_m_);
  return lock_protocol::OK;
}

lock_protocol::status
lock_server_cache::stat(lock_protocol::lockid_t lid, int &r)
{
  CTG_INFO("stat request\n");
  r = nacquire;
  return lock_protocol::OK;
}

} // namespace lock_server
} // namespace ctgfs
