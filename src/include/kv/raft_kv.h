// Authors: Chen Qian(qcdsr970209@gmail.com)

#pragma once
#include <brpc/server.h>
#include <kv/kv.h>
#include <raft/fsm.h>
#include <util/util.h>
#include <string>

namespace ctgfs {
namespace kv {

// RaftKv is a kv store implemented using raft consensus algorithm
// that including 2*N+1 nodes and can tolerant N nodes failed.
//
// Example:
class RaftKV : public KV {
 public:
  RaftKV(std::string name, int port, const std::string& initial_conf);
  ~RaftKV();
  // Given the key reference and value reference which you want to
  // put into the kv store, it will return bool to indicate success
  // or not.
  //
  // The function is re-entrant, so you can use it directly within
  // several threads.
  bool Put(const std::string& key, const std::string& value) override;

  // Given the key reference which you want to search within the kv
  // store, it will return bool to indicate sucess or not. If success,
  // the value will be the corresponding value. Otherwise, don't use
  // it.
  //
  // The function is re-entrant, so you can use it directly within
  // several threads.
  bool Get(const std::string& key, std::string& value) override;

  // Given the predicate and get the value lists, use this for mvcc_get
  //
  // The function is re-entrant, so you can use it directly within
  // several threads.
  bool Query(const std::string& key,
             std::map<std::string, std::string>& values) override;

 private:
  std::shared_ptr<raft::RocksFSM> fsm_;
  brpc::Server* server_;
};

}  // namespace kv
}  // namespace ctgfs
