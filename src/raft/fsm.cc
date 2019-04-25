/*
 * Authors: Chen Qian(qcdsr970209@gmail.com)
 */

#include <raft/fsm.h>

namespace ctgfs {
namespace raft {

RocksFSM::RocksFSM(const util::Options& options)
    : node_(nullptr), leader_term_(-1), options_(options), db_(nullptr) {}

RocksFSM::~RocksFSM() { Close(); }

util::Status RocksFSM::Open() {
  // TODO(Handora): use configurable options
  butil::EndPoint addr(butil::my_ip(), options_.local_port);
  braft::NodeOptions node_options;
  node_options.election_timeout_ms = options_.election_timeout_ms;
  node_options.fsm = this;
  node_options.initial_conf.parse_from(options_.initial_conf);
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
    return util::Status::Corruption("failed to init raft node");
  }
  node_ = node;

  rocksdb::Options rocks_options;
  rocks_options.create_if_missing = true;
  rocksdb::Status s =
      rocksdb::DB::Open(rocks_options, options_.rocksdb_path, &db_);
  if (!s.ok()) {
    return util::Status::Corruption("Can't open rocksdb " + s.ToString());
  }

  return util::Status::OK();
}

void RocksFSM::Close() {
  if (db_ != nullptr) {
    delete db_;
  }

  if (node_ != nullptr) {
    node_->shutdown(nullptr);
    // node_->join();
  }

  db_ = nullptr;
  node_ = nullptr;
}

bool RocksFSM::IsLeader() const {
  const int64_t term = leader_term_.load(butil::memory_order_relaxed);
  if (term < 0) {
    return false;
  }
  return true;
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
        break;
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
    waiter_->Signal();
  } else {
    waiter_->Signal(util::Status::Corruption(status().error_str()));
  }
}

util::Status RocksFSM::Put(const std::string& key, const std::string& value,
                           std::shared_ptr<util::Waiter> waiter) {
  return propose(OP_PUT, key, value, waiter);
}

util::Status RocksFSM::LocalGet(const std::string& key, std::string& value) {
  rocksdb::Status s = db_->Get(rocksdb::ReadOptions(), key, &value);
  if (!s.ok()) {
    return util::Status::Corruption(s.ToString());
  }
  return util::Status::OK();
}

util::Status RocksFSM::LocalQuery(const std::string& key, std::map<std::string, std::string>& values) {
  auto iter = db_->NewIterator(rocksdb::ReadOptions());
  for (iter->Seek(key); iter->Valid(); iter->Next()) {
    values.insert({iter->key().ToString(), iter->value().ToString()}); 
  }
  return util::Status::OK();
}

util::Status RocksFSM::Get(const std::string& key,
                           std::shared_ptr<util::Waiter> waiter) {
  return propose(OP_GET, key, "", waiter);
}

util::Status RocksFSM::put(const std::string& key, const std::string& value) {
  rocksdb::Status s = db_->Put(rocksdb::WriteOptions(), key, value);
  if (!s.ok()) {
    return util::Status::Corruption(s.ToString());
  }
  return util::Status::OK();
}

util::Status RocksFSM::propose(ProposeType type, const std::string& key,
                               const std::string& value,
                               std::shared_ptr<util::Waiter> waiter) {
  // const int64_t term = leader_term_.load(butil::memory_order_relaxed);
  // if (term < 0) {
  //   return util::Status::NotLeader("propose to nonleader");
  // }

  butil::IOBuf log;
  log.push_back((uint8_t)type);
  uint32_t size = (uint32_t)key.size();
  log.append(&size, sizeof(uint32_t));
  log.append(key);

  switch (type) {
    case OP_GET:
      break;
    case OP_PUT: {
      size = static_cast<uint32_t>(value.size());
      log.append(&size, sizeof(uint32_t));
      log.append(value);
    default:
      assert("unsupported");
    }
  }

  braft::Task task;
  task.data = &log;

  task.done = new RocksClosure(waiter);
  node_->apply(task);

  return util::Status::OK();
}

void RocksFSM::on_leader_start(int64_t term) {
  leader_term_.store(term, butil::memory_order_release);
}

void RocksFSM::on_leader_stop(const butil::Status& status) {
  leader_term_.store(-1, butil::memory_order_release);
}

void RocksFSM::on_error(const ::braft::Error& e) {}

void RocksFSM::on_configuration_committed(const ::braft::Configuration& conf) {}

void RocksFSM::on_stop_following(const ::braft::LeaderChangeContext& ctx) {}

void RocksFSM::on_start_following(const ::braft::LeaderChangeContext& ctx) {}

}  // namespace raft
}  // namespace ctgfs
