#pragma once

#include "rpc/rpc.h"

class master_protocol {
 public:
  typedef int status;
  enum xxstatus { OK, RPCERR, NOENT, IOERR };
  enum rpc_numbers {
    heart_beat = 0x7001,
    regist = 0x7002,
    ask_for_ino = 0x7003,
    ask_for_kv = 0x7004,
  };
};
