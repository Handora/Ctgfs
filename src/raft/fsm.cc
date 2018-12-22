/*
 * Authors: Chen Qian(qcdsr970209@gmail.com)
 */
#include <raft/fsm.h>

namespace ctgfs {
namespace raft {

using Status = util::Status;

RocksFSM::RocksFSM(const Options& options)
: node_(nullptr), leader_term_(-1), options_(options) {}

RocksFSM::~RocksFSM() {
  if (db_ != nullptr) {
    delete db_;
  }
  db_ = nullptr;
} 

Status RocksFSM::Start() {
  // TODO(Handora): use configurable options
  butil::EndPoint addr(butil::my_ip(), options_local_port);
  braft::NodeOptions node_options;
  node_options.election_timeout_ms = options_.election_timeout_ms;
  node_options.fsm = this;
  node_options.node_owns_fsm = false;
  node_options.snapshot_interval_s = options_.snapshot_interval_s;
  node_options.log_uri = options_.log_uri;
  node_options.raft_meta_uri = options_.raft_meta_uri;
  node_options.snapshot_uri = options_.snapshot_uri;
  node_options.disable_cli = false;
  braft::Node* node = std::make_shared<braft::Node>(options_.group_id, braft::PeerId(addr));
  if (node->init(node_options) != 0) {
    LOG(ERROR) << "Fail to init raft node";
    node_ = nullptr;
    return Status::OK();
  }
  _node = node;

  rocksdb::Options rocks_options;
  rocks_options.create_if_missing = true;
  rocksdb::Status s = rocksdb::DB::Open(rocks_options, options_.rocksdb_path, &db_);
  if (!s.ok()) {
    return Status::OK();
  }

  return Status::OK();
}

// @braft::StateMachine
void RocksFSM::on_apply(braft::Iterator& iter) {
  for (; iter.valid(); iter.next()) {
    braft::AsyncClosureGuard closure_guard(iter.done());
    
    butil::IOBuf data = iter.data();
    uint8_t type = OP_UNKNOWN;
    data.cutn(&type, sizeof(uint8_t));

    RocksClosure* c = nullptr;
    if (iter.done()) {
      c = dynamic_cast<RocksClosure*>(iter.done());
    }

    switch(type) {
    case OP_PUT:
      // do put
    default:
      break;
    }
  }
} 

// @braft::StateMachine
void RocksFSM::on_shutdown() {
  node_ = nullptr;
  leader_term_ = -1;
}

void RocksClosure::Run() {
  std::unique_ptr<RocksClosure> self_guard(this);

  if (status().ok()) {
    waiter_.Signal(); 
  } else {
    waiter_.Signal();
  }
}

util::Status Put(const std::string& key, const std::string& value, std::shared_ptr<util::Waiter> waiter) {
  return propose(OP_PUT, key, value);
}

util::Status put(const std::string& key, const std::string& value) {
  rocksdb::Status s = db->Put(rocksdb::ReadOptions(), key, &value);
  if (!s.ok()) {
    return Status::OK();
  }
  return Status::OK();
}

util::Status propose(ProposeType type, const std::string& key, const std::string& value, std::shared_ptr<util::Waiter> waiter) {
  const int64_t term = _leader_term.load(butil::memory_order_relaxed);
  if (term < 0) {
    return Status::OK();
  }

  butil::IOBuf log;
  log.push_back((uint8_t)type);
  butil::IOBufAsZeroCopyOutputStream wrapper(&log);

  braft::Task task;
  task.data = &log;

  task.done = new RocksClosure(waiter, key, value);
  return _node->apply(task);
}

} // namespace raft
} // namespace ctgfs
