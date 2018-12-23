// Authors: Chen Qian(qcdsr970209@gmail.com)

#pragma once
#include <braft/raft.h>
#include <braft/storage.h>
#include <braft/util.h>
#include <glog/logging.h>
#include <rocksdb/db.h>
#include <util/options.h>
#include <util/status.h>
#include <util/waiter.h>
#include <string>

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
  enum ProposeType { OP_UNKNOWN = 0, OP_GET, OP_PUT };

  RocksFSM(const util::Options& options);
  ~RocksFSM();

  // disable copy constructor and assignment onstructor
  // because RocksFSM is not torelanted to copy.
  //
  // While move is ok.
  RocksFSM& operator=(const RocksFSM& fsm) = delete;
  RocksFSM(const RocksFSM& fsm) = delete;

  // Start the RocksFSM, it create the node_ inside
  // the function which communicate with each other
  Status Open();
  void Close();

  Status Put(const std::string& key, const std::string& value,
             std::shared_ptr<util::Waiter> waiter);

 protected:
  void on_apply(braft::Iterator& iter) override;
  void on_shutdown() override;

 private:
  Status put(const std::string& key, const std::string& value);
  Status propose(ProposeType type, const std::string& key,
                 const std::string& value,
                 std::shared_ptr<util::Waiter> waiter);

  // TODO(Handora): why volatile
  std::shared_ptr<braft::Node> node_;
  butil::atomic<int64_t> leader_term_;
  util::Options options_;
  rocksdb::DB* db_;
};

// Implements Closure which encloses RPC stuff
class RocksClosure : public braft::Closure {
 public:
  RocksClosure(std::shared_ptr<util::Waiter> waiter) : waiter_(waiter) {}
  ~RocksClosure() {}

  void Run() override;

 private:
  std::shared_ptr<util::Waiter> waiter_;
};

}  // namespace raft
}  // namespace ctgfs
