/*
 * author: Chen Qian(qcdsr970209@gmail.com)
 **/
#pragma once
#include <string>
#include <rpc/rpc.h>
#include <mutex>
#include <memory>

namespace ctgfs {
namespace master {
// Global Timestamp Generator
class GTG {
 public:
  GTG(int port);
  ~GTG();
  int get_ts(int, int& r);
 public:
  static const int GET_TS = 6543; 
 private:
  int logical_;
  std::mutex mu_;
  std::shared_ptr<rpcs> rpc_serv_;
};
}  // namespace master
}  // namespace ctgfs
