/*
 * Authors: Chen Qian(qcdsr970209@gmail.com)
 */
#include <raft/fsm.h>

namespace ctgfs {
namespace raft {

using Status = util::Status;

RocksFSM::RocksFSM(const util::Options& options)
    : node_(nullptr), leader_term_(-1), options_(options) {}

RocksFSM::~RocksFSM() { Close(); }

Status RocksFSM::Open() {
  // TODO(Handora): use configurable options
  butil::EndPoint addr(butil::my_ip(), options_.local_port);
  braft::NodeOptions node_options;
  node_options.election_timeout_ms = options_.election_timeout_ms;
  node_options.fsm = this;
  node_options.node_owns_fsm = false;
  node_options.snapshot_interval_s = options_.snapshot_interval_s;
  node_options.log_uri = options_.log_uri;
  node_options.raft_meta_uri = options_.raft_meta_uri;
  node_options.snapshot_uri = options_.snapshot_uri;
  node_options.disable_cli = false;
  std::shared_ptr<braft::Node> node =
      std::make_shared<braft::Node>(options_.group_id, braft::PeerId(addr));
  if (node->init(node_options) != 0) {
    node_ = nullptr;
    return Status::Corruption("failed to init raft node");
  }
  node_ = node;

  rocksdb::Options rocks_options;
  rocks_options.create_if_missing = true;
  rocksdb::Status s =
      rocksdb::DB::Open(rocks_options, options_.rocksdb_path, &db_);
  if (!s.ok()) {
    return Status::Corruption("Can't open rocksdb");
  }

  return Status::OK();
}

void RocksFSM::Close() {
  if (db_ != nullptr) {
    delete db_;
  }
  db_ = nullptr;
}

// @braft::StateMachine
void RocksFSM::on_apply(braft::Iterator& iter) {
  // apply from iter one by one
  for (; iter.valid(); iter.next()) {
    braft::AsyncClosureGuard closure_guard(iter.done());

    butil::IOBuf data = iter.data();
    uint8_t type = OP_UNKNOWN;
    data.cutn(&type, sizeof(uint8_t));

    switch (type) {
      case OP_PUT: {
        uint32_t size;
        std::string key, value;
        data.cutn(&size, sizeof(uint32_t));
        data.cutn(&key, size);

        data.cutn(&size, sizeof(uint32_t));
        data.cutn(&value, size);
        put(key, value);
      }
      default:
        break;
    }
  }
}

// @braft::StateMachine
void RocksFSM::on_shutdown() {
  node_ = nullptr;
  leader_term_ = -1;
  Close();
}

void RocksClosure::Run() {
  std::unique_ptr<RocksClosure> self_guard(this);

  if (status().ok()) {
    waiter_->Signal(util::Status::Corruption(status().error_str()));
  } else {
    waiter_->Signal();
  }
}

util::Status RocksFSM::Put(const std::string& key, const std::string& value,
                           std::shared_ptr<util::Waiter> waiter) {
  return propose(OP_PUT, key, value, waiter);
}

util::Status RocksFSM::put(const std::string& key, const std::string& value) {
  rocksdb::Status s = db_->Put(rocksdb::WriteOptions(), key, value);
  if (!s.ok()) {
    return Status::Corruption(s.ToString());
  }
  return Status::OK();
}

util::Status RocksFSM::propose(ProposeType type, const std::string& key,
                               const std::string& value,
                               std::shared_ptr<util::Waiter> waiter) {
  const int64_t term = leader_term_.load(butil::memory_order_relaxed);
  // if (term < 0) {
  //   return Status::NotLeader("propose to nonleader");
  // }

  butil::IOBuf log;
  log.push_back((uint8_t)type);
  log.push_back((uint32_t)(key.size()));
  log.append(key);
  log.push_back((uint32_t)(value.size()));
  log.append(value);

  braft::Task task;
  task.data = &log;

  task.done = new RocksClosure(waiter);
  node_->apply(task);

  return util::Status::OK();
}

}  // namespace raft
}  // namespace ctgfs
