// Authors: Chen Qian(qcdsr970209@gmail.com)

#pragma once
#include <string>
#include <braft/raft.h>
#include <braft/storage.h>
#include <glog/logging.h>
#include <util/status.h>

namespace ctgfs {
namespace raft {

using Status = util::Status;
#define DEFAULT_PORT 51202
#define DEFAULT_ELECTION_TIMEOUT_MS 3000
#define DEFAULT_SNAPSHOT_INTERVAL_S 60 * 60 * 24
#define DEFAULT_DISABLE_CLI true
#define DEFAULT_EXAMPLE_GROUP "example"

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
  Status Start();

 protected:
  // TODO(Handora): why volatile
  std::shared_ptr<braft::Node> node_;
  butil::atomic<int64_t> leader_term_;

  void on_apply(braft::Iterator& iter) override;
  void on_shutdown() override;
};

} // namespace kv
} // namespace ctgfs
