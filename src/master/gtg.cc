/*
* author: Chen Qian(qcdsr970209@gmail.com)
**/
#include <master/gtg.h>

namespace ctgfs {
namespace master {

GTG::GTG(int port) : logical_(0) {
  rpc_serv_ = std::make_shared<rpcs>(port, 0);
  rpc_serv_->reg(GET_TS, this, &GTG::get_ts);
}

GTG::~GTG() {}

int GTG::get_ts(int, int &r) {
  std::lock_guard<std::mutex> guard(mu_);
  r = logical_++;
  return 0;
}

}  // namespace master
}  // namespace ctgfs
