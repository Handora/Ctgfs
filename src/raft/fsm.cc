/*
 * Authors: Chen Qian(qcdsr970209@gmail.com)
 */
#include <raft/fsm.h>

namespace ctgfs {
namespace raft {

using Status = util::Status;

RocksFSM::RocksFSM()
  : node_(nullptr), leader_term_(-1) {}

RocksFSM::~RocksFSM() {} 

Status RocksFSM::Start() {
  // TODO(Handora): use configurable options
  butil::EndPoint addr(butil::my_ip(), DEFAULT_PORT);
  braft::NodeOptions node_options;
  node_options.election_timeout_ms = DEFAULT_ELECTION_TIMEOUT_MS;
  node_options.fsm = this;
  node_options.node_owns_fsm = false;
  node_options.snapshot_interval_s = DEFAULT_SNAPSHOT_INTERVAL_S;
  std::string prefix = "/tmp/ctgfs";
  node_options.log_uri = prefix + "/log";
  node_options.raft_meta_uri = prefix + "/raft_meta";
  node_options.snapshot_uri = prefix + "/snapshot";
  node_options.disable_cli = DEFAULT_DISABLE_CLI;
  braft::Node* node = new braft::Node(DEFAULT_EXAMPLE_GROUP, braft::PeerId(addr));
 if (node->init(node_options) != 0) {
     LOG(ERROR) << "Fail to init raft node";
     delete node;
     return Status::OK();
 }
 _node = node;
 return Status::OK();
}

// @braft::StateMachine
void on_apply(braft::Iterator& iter) {
  for (; iter.valid(); iter.next()) {
    braft::AsyncClosureGuard closure_guard(iter.done());
    
  }
} 

} // namespace raft
} // namespace ctgfs
