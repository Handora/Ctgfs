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
  util::Status Open();

  // Close the fsm, including close the raftnode and
  // rocksdb inside it
  void Close();

  // Put the key into the value, it will be replicated 
  // to majority of all nodes. you can use the waiter
  // to wait for put applied or asynchronously to let
  // it applied
  util::Status Put(const std::string& key, const std::string& value,
             std::shared_ptr<util::Waiter> waiter);

  util::Status Get(const std::string& key, std::string& value,
             std::shared_ptr<util::Waiter> waiter);

  bool IsLeader() const;

 protected:
  // what to do when the value is applied to Rocksdb
  void on_apply(braft::Iterator& iter) override;
  // what to do when the statemachine is shutdown
  void on_shutdown() override;

  void on_leader_start(int64_t term) override;
  void on_leader_stop(const butil::Status& status) override;
  void on_error(const braft::Error& e) override;
  void on_configuration_committed(const braft::Configuration& conf) override;
  void on_stop_following(const braft::LeaderChangeContext& ctx) override;
  void on_start_following(const braft::LeaderChangeContext& ctx) override;

 private:
  // propose the `PUT` type log
  util::Status put(const std::string& key, const std::string& value);

  // propose the log and let raft to replicate and commit it
  util::Status propose(ProposeType type, const std::string& key,
                 const std::string& value,
                 std::shared_ptr<util::Waiter> waiter);

  // TODO(Handora): should we use volatile
  std::shared_ptr<braft::Node> node_;
  butil::atomic<int64_t> leader_term_;
  util::Options options_;
  rocksdb::DB* db_;
};

// Implements Closure which encloses RPC stuff
class RocksClosure : public braft::Closure {
 public:
  RocksClosure(std::shared_ptr<util::Waiter> waiter) 
      : waiter_(waiter) {}
  ~RocksClosure() {}

  // notify the waiter, should we do some else?
  void Run() override;

 private:
  std::shared_ptr<util::Waiter> waiter_;
};

}  // namespace raft
}  // namespace ctgfs
