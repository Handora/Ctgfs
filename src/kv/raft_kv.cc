// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <kv/raft_kv.h>

namespace ctgfs {
namespace kv {

RaftKV::RaftKV(std::string name, int port, const std::string& initial_conf) {
  util::Options options(name, port);
  options.initial_conf = initial_conf;
  options.log_uri = "local://tmp/fsm_" + name + "_" + std::to_string(port) + "/log";
  options.raft_meta_uri =
        "local://tmp/fsm_" + name + "_" + std::to_string(port) + "/raft_meta";
  options.snapshot_uri = "local://tmp/fsm_" + name + "_" + std::to_string(port) + "/snapshot";
  options.rocksdb_path = "tmp/rocksdb_" + name + "_" + std::to_string(port);

  server_ = new brpc::Server();
  fsm_ = std::make_shared<raft::RocksFSM>(options);
  // TODO: check result
  int res = braft::add_service(server_, port);
  std::cout << res << std::endl;
  assert(0 == res);

  util::Status s = fsm_->Open();
  assert(s.IsOK());
  res = server_->Start(port, NULL);
  assert(0 == res);
}

RaftKV::~RaftKV() {
  fsm_->Close();
  delete server_;
}

bool RaftKV::Put(const std::string& key, const std::string& value) {
  // TODO: use waiter for kv interface
  bool ret = true;
  if (fsm_->IsLeader()) {
    auto waiter = std::make_shared<util::Waiter>();
    util::Status s = fsm_->Put(key, value, waiter);
    if (!s.IsOK()) {
      ret = false;
    } else {
      s = waiter->Wait();
      if (!s.IsOK()) {
        ret = false;
      }
    }
  } else {
    ret = false;
  }

  return ret;
}

bool RaftKV::Get(const std::string& key, std::string& value) {
  // TODO: use waiter for kv interface
  bool ret = true;
  util::Status s = fsm_->LocalGet(key, value);
  if (!s.IsOK()) {
    ret = false;
  }

  return ret;
}

bool RaftKV::Query(const std::string& key, std::map<std::string, std::string>& value) {
  bool ret = true;
  std::map<std::string, std::string> tmp;
  util::Status s = fsm_->LocalQuery(key, tmp);
  if (!s.IsOK()) {
    ret = false;
  }

  for (auto it = tmp.begin(); it != tmp.end(); it++) {
    std::size_t found = it->first.find("_");
    if (found != std::string::npos) {
      value.insert({it->first, it->second});
    }
  }

  return ret;
}
}  // namespace kv
}  // namespace ctgfs
