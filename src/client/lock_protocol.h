// lock protocol

#ifndef lock_protocol_h
#define lock_protocol_h

#include "rpc/rpc.h"

class lock_protocol {
 public:
  enum xxstatus { OK, RETRY, RPCERR, NOENT, IOERR, RETIRE };
  typedef int status;
  typedef unsigned long long lockid_t;
  typedef unsigned long long xid_t;
  enum rpc_numbers {
    acquire = 0x7001,
    release,
    stat
  };
};

class rlock_protocol {
 public:
  enum xxstatus { OK, RPCERR, RETRY, WAIT };
  typedef int status;
  enum rpc_numbers {
    revoke = 0x8001,
    retry = 0x8002
  };
};

enum lock_state {
  NONE = 0, // client knows nothing about this lock
  FREE, // client owns the lock and no thread has it
  LOCKED, // client owns the lock and a thread has it
  ACQUIRING, // the client is acquiring ownership
  RELEASING, // the client is releasing ownership
};

struct lock_cache {
  lock_state state;
  pthread_cond_t* cond;
  bool is_revoked;

  lock_cache(lock_state st=NONE): state(st), is_revoked(false) {
    cond = new pthread_cond_t();
    VERIFY(pthread_cond_init(cond, 0) == 0);
  }
};

struct lock_serv_state {
  std::string holder;
  bool holded;
  std::vector<std::string> pendingers;

  lock_serv_state(std::string hold)
    :holder(hold), holded(true) {}

  lock_serv_state()
    :holder(""), holded(false) {}
};
#endif 
