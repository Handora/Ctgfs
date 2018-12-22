// Authors: Chen Qian(qcdsr970209@gmail.com)

#pragma once
#include <string>
#include <braft/raft.h>
#include <braft/storage.h>
#include <glog/logging.h>
#include <util/status.h>
#include <util/options.h>
#include <util/waiter.h>
#include <rocksdb/db.h>

namespace ctgfs {
namespace raft {

using Status = util::Status;

// The finite state machine powered by rocksdb and braft
// Provides distributed fault torelant and KV storage
// interface instance
//
// Example:
//
class RocksFSM : public braft::StateMachine {
 public:
  
  RocksFSM(const Options& options);
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

  Status Put(const std::string& key, const std::string& value);

 protected:
  void on_apply(braft::Iterator& iter) override;
  void on_shutdown() override;

 private:
  Status put(const std::string& key, const std::string& value);

  // TODO(Handora): why volatile
  std::shared_ptr<braft::Node> node_;
  butil::atomic<int64_t> leader_term_;
  Options options_;
  rocksdb::DB* db_;
};

// Implements Closure which encloses RPC stuff
class RocksClosure : public braft::Closure {
 public:
  RocksClosure(std::shared_ptr<util::Waiter> waiter)
    : waiter_(waiter)
  ~RocksClosure() {}

  void Run() override;

private:
  std::shared_ptr<util::Waiter> waiter_;
};

} // namespace kv
} // namespace ctgfs
