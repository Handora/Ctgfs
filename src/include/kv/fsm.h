// Authors: Chen Qian(qcdsr970209@gmail.com)

#pragma once
#include <string>
#include <braft/raft.h>
#include <braft/storage.h>
#include <glog/logging.h>

namespace ctgfs {
namespace kv {

// The finite state machine powered by rocksdb and braft
// Provides distributed fault torelant and KV storage
// interface instance
//
// Example:
//
class RocksFSM : public braft::StateMachine {
 public:
  
  RocksFSM();
  ~RocksFSM();

  // disable copy constructor and assignment onstructor
  // because RocksFSM is not torelanted to copy.
  //
  // While move is ok.
  RocksFSM& operator=(const RocksFSM& fsm) = delete;
  RocksFSM(const RocksFSM& fsm) = delete;

  // Start the RocksFSM, it create the node_ inside
  // the function which communicate with each other 
  int Start();

 protected:
  // TODO(Handora): why volatile
  braft::Node * volatile node_;
  butil::atomic<int64_t> leader_term_;

  void on_apply(braft::Iterator& iter);
  void on_shutdown();
};

} // namespace kv
} // namespace ctgfs
