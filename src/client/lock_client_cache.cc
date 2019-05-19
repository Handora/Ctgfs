// RPC stubs for clients to talk to lock_server, and cache the locks
// see lock_client.cache.h for protocol details.

#include "client/lock_client_cache.h"
#include <stdio.h>
#include <iostream>
#include <sstream>
#include "client/tprintf.h"
#include "rpc/rpc.h"

namespace ctgfs {
namespace client {

lock_client_cache::lock_client_cache(std::string xdst,
                                     class lock_release_user *_lu)
    : lock_client(xdst), lu(_lu), waiting_threads(0) {
  VERIFY(pthread_mutex_init(&lock_m_, 0) == 0);

  rpcs *rlsrpc = new rpcs(0);
  rlsrpc->reg(rlock_protocol::revoke, this, &lock_client_cache::revoke_handler);
  rlsrpc->reg(rlock_protocol::retry, this, &lock_client_cache::retry_handler);

  const char *hname;
  hname = "127.0.0.1";
  std::ostringstream host;
  host << hname << ":" << rlsrpc->port();
  id = host.str();
}

std::string print_state(int state) {
  switch (state) {
    case NONE:
      return "NONE";
    case LOCKED:
      return "LOCKED";
    case ACQUIRING:
      return "ACQUIRING";
    case RELEASING:
      return "RELEASING";
    case FREE:
      return "FREE";
  }
  return "UNKNOWN";
}

lock_protocol::status lock_client_cache::acquire(lock_protocol::lockid_t lid) {
  lock_protocol::status ret = lock_protocol::OK;
  int r = rlock_protocol::OK;

  pthread_mutex_lock(&lock_m_);
  if (lsm_.find(lid) == lsm_.end()) {
    lsm_[lid] = lock_cache();
  }

  while (true) {
    tprintf("Client %s acquire lock %llu in state %s\n", id.c_str(), lid,
            print_state(lsm_[lid].state).c_str());
    switch (lsm_[lid].state) {
      case NONE:
      case RELEASING:
        // TODO(Handora): HOW TO DO RELEASING MORE SEPHISITICLY
        {
          lsm_[lid].state = ACQUIRING;
          pthread_mutex_unlock(&lock_m_);

          ret = cl->call(lock_protocol::acquire, lid, id, r);

          if (ret != lock_protocol::OK) {
            return ret;
          }

          pthread_mutex_lock(&lock_m_);
          tprintf(
              "Client %s get back from acquire rpc with lock %llu in state "
              "%s\n",
              id.c_str(), lid, print_state(lsm_[lid].state).c_str());

          if (lsm_[lid].state != ACQUIRING) {
            continue;
          }

          if (r == lock_protocol::RETRY) {
            lsm_[lid].state = NONE;
            tprintf(
                "Client %s get back from acquire rpc with lock %llu and need "
                "wait for retry\n",
                id.c_str(), lid);
            waiting_threads++;
            pthread_cond_wait(lsm_[lid].cond, &lock_m_);
            waiting_threads--;
            tprintf("Client %s with lock %llu wake up and restart acquire\n",
                    id.c_str(), lid);
            continue;
          }

          if (lsm_[lid].is_revoked) {
            tprintf("Client %s get back from acquire rpc while is revoked\n",
                    id.c_str());
            lsm_[lid].is_revoked = false;
            lsm_[lid].state = NONE;
            continue;
          }

          lsm_[lid].state = LOCKED;
          if (r == lock_protocol::RETIRE) {
            lsm_[lid].is_revoked = true;
          }
          break;
        }
      case FREE: {
        lsm_[lid].state = LOCKED;
        break;
      }
      case LOCKED:
      case ACQUIRING: {
        tprintf("Client %s's node is acquiring, so i can wait\n", id.c_str());
        waiting_threads++;
        pthread_cond_wait(lsm_[lid].cond, &lock_m_);
        waiting_threads--;
        tprintf("Client %s with lock %llu wake up and restart acquire\n",
                id.c_str(), lid);
        continue;
      }
    }

    break;
  }

  tprintf("Client %s get lock %llu happily\n", id.c_str(), lid);
  pthread_mutex_unlock(&lock_m_);
  return lock_protocol::OK;
}

lock_protocol::status lock_client_cache::release(lock_protocol::lockid_t lid) {
  lock_protocol::status ret = lock_protocol::OK;
  bool need_send = false;

  pthread_mutex_lock(&lock_m_);
  if (lsm_.find(lid) == lsm_.end()) {
    lsm_[lid] = lock_cache(NONE);
  }
  tprintf("Client %s release lock %llu in state %s\n", id.c_str(), lid,
          print_state(lsm_[lid].state).c_str());

  switch (lsm_[lid].state) {
    case NONE:
    case FREE: {
      // should never happen
      tprintf("Never meet release when in none/free state\n");
      assert(0);
    }
    case LOCKED: {
      if (lsm_[lid].is_revoked) {
        tprintf("Client %s release lock and in revoked state\n", id.c_str());
        lsm_[lid].is_revoked = false;
        int r = 0;
        lsm_[lid].state = RELEASING;
        pthread_mutex_unlock(&lock_m_);

        lu->dorelease(lid);
        ret = cl->call(lock_protocol::release, lid, id, r);
        if (ret != lock_protocol::OK) {
          return ret;
        }

        pthread_mutex_lock(&lock_m_);
        tprintf(
            "Client %s get release rpc back when release lock %llu in state "
            "%s\n",
            id.c_str(), lid, print_state(lsm_[lid].state).c_str());

        if (lsm_[lid].state == RELEASING) {
          lsm_[lid].state = NONE;
          if (waiting_threads != 0) {
            need_send = true;
          }
        }
      } else {
        tprintf("Client %s release lock %llu and signal others\n", id.c_str(),
                lid);

        lsm_[lid].state = FREE;
        pthread_cond_signal(lsm_[lid].cond);
      }
      break;
    }
    case RELEASING: {
      // should never happen
      tprintf("Never meet release when in releasing state\n");
      assert(0);
    }
    case ACQUIRING: {
      // should never happen
      tprintf("Never meet release when in acquiring state\n");
      assert(0);
    }
  }

  if (need_send) {
    lsm_[lid].state = ACQUIRING;
    pthread_mutex_unlock(&lock_m_);

    int r = 0;
    ret = cl->call(lock_protocol::acquire, lid, id, r);

    if (ret != lock_protocol::OK) {
      return ret;
    }

    pthread_mutex_lock(&lock_m_);
    tprintf(
        "Client %s get back from acquire for need_send of release with lock "
        "%llu in state %s\n",
        id.c_str(), lid, print_state(lsm_[lid].state).c_str());

    if (lsm_[lid].state == ACQUIRING) {
      if (r == lock_protocol::RETRY) {
        lsm_[lid].state = NONE;
      } else {
        lsm_[lid].state = FREE;
        pthread_cond_signal(lsm_[lid].cond);
        if (r == lock_protocol::RETIRE) lsm_[lid].is_revoked = true;
      }
    }
  }

  tprintf("Client %s release lock %llu happily\n", id.c_str(), lid);
  pthread_mutex_unlock(&lock_m_);
  return lock_protocol::OK;
}

rlock_protocol::status lock_client_cache::revoke_handler(
    lock_protocol::lockid_t lid, int &) {
  rlock_protocol::status ret = rlock_protocol::OK;
  int r;

  pthread_mutex_lock(&lock_m_);
  if (lsm_.find(lid) == lsm_.end()) {
    lsm_[lid] = lock_cache(NONE);
  }
  tprintf("Client %s get revoke rpc with lock %llu in state %s\n", id.c_str(),
          lid, print_state(lsm_[lid].state).c_str());

  switch (lsm_[lid].state) {
    case NONE:
    case RELEASING: {
      // TODO(Handora): should it happen?
      break;
    }
    case FREE: {
      lsm_[lid].state = RELEASING;
      tprintf(
          "Client %s get revoke rpc with lock %llu in FREE and start to "
          "release\n",
          id.c_str(), lid);
      pthread_mutex_unlock(&lock_m_);
      lu->dorelease(lid);
      ret = cl->call(lock_protocol::release, lid, id, r);
      if (ret != rlock_protocol::OK) {
        return ret;
      }

      pthread_mutex_lock(&lock_m_);
      tprintf(
          "Client %s get release rpc for revoke back with lock %llu in state "
          "%s\n",
          id.c_str(), lid, print_state(lsm_[lid].state).c_str());
      if (lsm_[lid].state == RELEASING) lsm_[lid].state = NONE;
      break;
    }
    case LOCKED: {
      tprintf(
          "Client %s is locking for lock %llu. so we set tags and wait for the "
          "task to be finished\n",
          id.c_str(), lid);
      lsm_[lid].is_revoked = true;
      break;
    }
    case ACQUIRING: {
      printf("Client %s should remeber the revoking for lock %llu.\n",
             id.c_str(), lid);
      lsm_[lid].is_revoked = true;
      break;
    }
  }

  pthread_mutex_unlock(&lock_m_);
  return ret;
}

rlock_protocol::status lock_client_cache::retry_handler(
    lock_protocol::lockid_t lid, int &r) {
  rlock_protocol::status ret = rlock_protocol::OK;

  pthread_mutex_lock(&lock_m_);
  if (lsm_.find(lid) == lsm_.end()) {
    assert(0);
    lsm_[lid] = lock_cache();
  }

  tprintf("Client %s get retry rpc with lock %llu in state %s\n", id.c_str(),
          lid, print_state(lsm_[lid].state).c_str());

  while (true) {
    switch (lsm_[lid].state) {
      case NONE:
      case ACQUIRING:
      case RELEASING: {
        lsm_[lid].state = ACQUIRING;
        pthread_mutex_unlock(&lock_m_);

        ret = cl->call(lock_protocol::acquire, lid, id, r);

        if (ret != lock_protocol::OK) {
          return ret;
        }

        pthread_mutex_lock(&lock_m_);
        tprintf(
            "Client %s get back from acquire for retry with lock %llu in state "
            "%s\n",
            id.c_str(), lid, print_state(lsm_[lid].state).c_str());

        if (lsm_[lid].state != ACQUIRING) continue;

        if (r == lock_protocol::RETIRE || r == lock_protocol::RETRY)
          lsm_[lid].is_revoked = true;

        lsm_[lid].state = FREE;
        pthread_cond_signal(lsm_[lid].cond);
        break;
      }
      case FREE:
      case LOCKED: {
        // TODO(Handora): should it happen?
        break;
      }
    }

    break;
  }

  pthread_mutex_unlock(&lock_m_);
  return ret;
}

void handle_none() {}

}  // namespace client
}  // namespace ctgfs
