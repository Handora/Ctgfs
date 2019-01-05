/*
* author: OneDay_(ltang970618@gmail.com)
**/
#include <butil/logging.h>
#include <master/heart_beat.h>
#include <master/master.h>

namespace ctgfs {
namespace master {

Master::Master() {
  for (int i = 0; i < 26; i++) {
    VALID_CHAR_SET.push_back('a' + i);
    VALID_CHAR_SET.push_back('A' + i);
  }
}

Master::~Master() {}

void Master::ClientAskForKV(::google::protobuf::RpcController* controller,
                            const ::ctgfs::ClientKVRequest* request,
                            ::ctgfs::ClientKVResponse* response,
                            ::google::protobuf::Closure* done) {
  brpc::ClosureGuard done_guard(done);
  if (cur_register_kv_id_ == 0) {
    LOG(ERROR) << "NO FS CONNECT" << std::endl;
    return;
  }
  // brpc::Controller* ctrl = static_cast<brpc::Controller*>(controller);
  setKVAddrByClientKVRequest(request, response);
}

void Master::UpdateKVInfo(
    const std::shared_ptr<ctgfs::heart_beat::HeartBeatInfo> info) {
  if (info->type == HeartBeatMessageRequest_HeartBeatType::
                        HeartBeatMessageRequest_HeartBeatType_kRegist) {
    if (!registerKV(info->addr)) return;
  }
  if (info->type == HeartBeatMessageRequest_HeartBeatType::
                        HeartBeatMessageRequest_HeartBeatType_kInfoUpdate ||
      info->type == HeartBeatMessageRequest_HeartBeatType::
                        HeartBeatMessageRequest_HeartBeatType_kRegist) {
    int id = addr_to_register_id_[info->addr];
    kv_info_[id] = info;
  }
}

void Master::setKVAddrByClientKVRequest(const ::ctgfs::ClientKVRequest* request,
                                        ::ctgfs::ClientKVResponse* response) {
  auto command = request->command();
  const std::string& path = command.path();
  int tar_kv_id = hashPath(path);
  std::string addr = register_id_to_addr_.at(tar_kv_id);
  response->set_id(request->id());
  response->set_addr(addr);
}

bool Master::registerKV(const std::string& ip, const int& port) {
  std::string addr(ip.begin(), ip.end());
  std::string port_str = std::to_string(port);
  addr += std::string(":") + port_str;
  return registerKV(addr);
}

bool Master::registerKV(const std::string& addr) {
  if (addr_to_register_id_.find(addr) != addr_to_register_id_.end()) {
    debugRegisterKV(true, "Duplicated Regist KV\n");
    return false;
  }
  int nxt_id = getNewRegisterID();
  // LOG(INFO) << "ID : " << nxt_id << std::endl;
  if (nxt_id == -1) {
    debugRegisterKV(true, "Generate Registe ID Error\n");
    return false;
  }
  doRegister(addr, nxt_id);
  return true;
}

int Master::getNewRegisterID() {
  // LOG(INFO) << "generating regist ID" << std::endl;
  if (!reused_queue_.empty()) {
    auto id = reused_queue_.front();
    reused_queue_.pop();
    kv_info_[id] = std::make_shared<heart_beat::HeartBeatInfo>();
    return id;
  }
  kv_info_.push_back(std::make_shared<heart_beat::HeartBeatInfo>());
  return cur_register_kv_id_++;
}

void Master::doRegister(const std::string& addr, const int& register_id) {
  register_id_to_addr_[register_id] = addr;
  addr_to_register_id_[addr] = register_id;
  debugRegisterKV(false, "Register Successfully");
}

int Master::hashPath(const std::string& path) {
  if (path.empty()) return 0;
  size_t split_pos = 0;
  int hash_val = 0;
  // auto split_pos = path.find_first_not_of(INVALID_CHAR_SET);
  while (1) {
    size_t nxt_pos = path.find_first_not_of(VALID_CHAR_SET, split_pos);
    if (nxt_pos == std::string::npos) {
      std::string cur_str(path.begin() + split_pos, path.end());
      hash_val += hashStr(cur_str);
      hashValueToRegisterID(hash_val);
      break;
    }
    std::string cur_str(path.begin() + split_pos, path.begin() + nxt_pos);
    hash_val += hashStr(cur_str);
    hashValueToRegisterID(hash_val);
    split_pos = nxt_pos + 1;
    if (split_pos >= path.size()) break;
  }
  return hash_val;
}

int Master::hashStr(const std::string& str) {
  int sum = 0;
  for_each(str.begin(), str.end(), [&](char x) {
    sum *= 26;
    sum += (x - '0');
    hashValueToRegisterID(sum);
  });
  return sum;
}

void Master::hashValueToRegisterID(int& val) {
  if (val >= cur_register_kv_id_ || val <= -cur_register_kv_id_)
    val %= cur_register_kv_id_;
  if (val < 0) val += cur_register_kv_id_;
}

void Master::unregisterKV(const int& regist_id) { doUnregister(regist_id); }

void Master::doUnregister(const int& register_id) {
  if (register_id_to_addr_.find(register_id) == register_id_to_addr_.end())
    return;
  reused_queue_.push(register_id);
  std::string addr = register_id_to_addr_[register_id];
  register_id_to_addr_.erase(register_id_to_addr_.find(register_id));
  addr_to_register_id_.erase(addr_to_register_id_.find(addr));
}

void Master::debugRegisterKV(bool is_error, const std::string& str) {
  if (is_error) {
    // wait to update with glog
  } else {
    // wait to update with glog
  }
}

void Master::debugRegisterKV(bool is_error, const char* str) {
  debugRegisterKV(is_error, std::string(str));
}

}  // namespace master
}  // namespace ctgfs
