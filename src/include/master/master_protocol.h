#pragma once

#include "rpc/rpc.h"

class master_protocol {
 public:
  typedef int status;
  enum xxstatus { OK, RPCERR, NOENT, IOERR };
  enum rpc_numbers {
    heart_beat = 0x7001,
  };
};