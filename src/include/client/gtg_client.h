// Author: Chen Qian(qcdsr970209@gmail.com)
// gtg client interface.

#pragma once

#include <string>
#include <vector>
#include "rpc/rpc.h"

// Client interface to the gtg
namespace ctgfs {
namespace client {

class GTC {
 protected:
  rpcc *cl_;
 public:
  GTC(std::string dst);
  virtual ~GTC() {};
  virtual int get_ts(int& ts);
};

}  // namespace client
}  // namespace ctgfs
