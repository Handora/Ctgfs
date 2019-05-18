/*
* author: OneDay_(ltang970618@gmail.com)
**/
#include <master/master.h>
#include "rpc/rpc.h"
#include "fs/extent_protocol.h"
#include "client/extent_client.h"
#include "client/extent_client_cache.h"
#include "client/lock_client.h"
#include "client/lock_client_cache.h"
#include <iostream>
#include "master/master_protocol.h"
#include <algorithm>

namespace ctgfs {
namespace master {
using namespace ::ctgfs::prefix_tree;
Master::Master() {
  for (int i = 0; i < 26; i++) {
    VALID_CHAR_SET.push_back('a' + i);
    VALID_CHAR_SET.push_back('A' + i);
  }
  t_ = std::make_shared<PrefixTree>();
  adjust_thread_ = std::thread([&](){
    while(!stop_) {
      auto score = calculateScore(t_->GetAdjustContext());
      auto move_op = t_->Adjust(score);
      sort(move_op.begin(), move_op.end(), [](std::pair<unsigned long long, std::pair<int, int> > a, std::pair<unsigned long long, std::pair<int, int> > b) {
        return a.second < b.second;
      });
      if(move_op.empty()) {
        continue;
      }
      std::vector<unsigned long long> pip_line;
      pip_line.push_back(move_op[0].first);
      std::pair<int, int> op_addr = move_op[0].second;
      for(auto i = 1; i < (int)move_op.size(); i ++) {
        inum_to_register_id_[move_op[i].first] = move_op[i].second.second;
        if(move_op[i].second == op_addr) {
          pip_line.push_back(move_op[i].first);
        }
        else {
          auto lock_addr = "";
          Move(lock_addr, pip_line, register_id_to_addr_[op_addr.first], register_id_to_addr_[op_addr.second]);
          pip_line.clear();
          pip_line.push_back(move_op[i].first);
          op_addr = move_op[i].second;
        }
      }
      if(!pip_line.empty()) {
        auto lock_addr = "";
        Move(lock_addr, pip_line, register_id_to_addr_[op_addr.first], register_id_to_addr_[op_addr.second]);
      }
      std::this_thread::sleep_for(std::chrono::seconds(3));
    }
  });
}

Master::~Master() {
  stop_ = true;
  adjust_thread_.join();
}

master_protocol::status Master::AskForIno(std::string path, bool is_dir, unsigned long long node_sz, std::pair<unsigned long long, std::string>& r) {
  auto ino = genInum(path, is_dir, node_sz);
  r.first = ino;
  r.second = getInfoByInum(ino);
  // TODO: need add more options
  return master_protocol::OK;
}

master_protocol::status Master::AskForKV(unsigned long long ino, std::string& r) {
  // unsigned long long ino = request->ino();
  // response->set_addr(getInfoByInum(ino));
  r = getInfoByInum(ino);
  printf("ask for kv: %016llu, resp %s\n",ino, r.c_str());
  return master_protocol::OK;
}

master_protocol::status Master::Regist(std::string addr, unsigned long long sum_memory, int& r) {
  auto id = getNewRegisterID();
  doRegister(addr, id);
  t_->RegistNewKV(id, sum_memory);
  r = id;
  return master_protocol::OK;
}

/*
void Master::SendHeartBeat(::google::protobuf::RpcController* controller,
                           const ::ctgfs::HeartBeatMessageRequest* request,
                           ::ctgfs::HeartBeatMessageResponse* response,
                           ::google::protobuf::Closure* done) {
  brpc::ClosureGuard done_guard(done);
  printf("In Send Heart Beat\n");
  // empty resp now so needn't fill
  // use rq to generate struct HeartBeatInfo
  // call the update method
  std::shared_ptr<HeartBeatInfo> info_ptr = std::make_shared<HeartBeatInfo>();
  info_ptr->addr = request->addr();
  info_ptr->type = request->type();
  updateKVInfo(info_ptr);
}

void Master::updateKVInfo(
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
*/

master_protocol::status Master::UpdateKVInfo(InfoCollector::ServerInfo i, int&) {
  /* when this function is called by rpc, the latest info of extent_server will be passed. */
  std::cout << "called by rpc:\n" << "num: " << i.file_num <<  ", size: " << i.disk_usage
  << std::endl;

  return master_protocol::OK;
}

/*
bool Master::registerKV(const std::string& ip, const int& port) {
  std::string addr(ip.begin(), ip.end());
  std::string port_str = std::to_string(port);
  addr += std::string(":") + port_str;
  return registerKV(addr);
}
*/

/*
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
*/

int Master::getNewRegisterID() {
  // LOG(INFO) << "generating regist ID" << std::endl;
  std::unique_lock<std::mutex> lck(regist_lock_);
  if (!reused_queue_.empty()) {
    auto id = reused_queue_.front();
    reused_queue_.pop();
    // kv_info_[id] = std::make_shared<heart_beat::HeartBeatInfo>();
    return id;
  }
  // kv_info_.push_back(std::make_shared<heart_beat::HeartBeatInfo>());
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

unsigned long long Master::genInum(const std::string& path, bool is_dir, unsigned long long node_sz) {
  std::lock_guard<std::mutex> locker(ino_count_mutex_);
  unsigned long long id = ino_count_ ++; 
  // id += random() % (2147483648);
  if (!is_dir) id |= 0x80000000;
  int kv_id;
  t_->Create(path, id, is_dir, node_sz, kv_id);
  inum_to_path_[id] = path;
  inum_to_register_id_[id] = kv_id;
  printf("gen new inum: %016x, path: %s, kv_id: %d, sz: %llu\n", id, path.c_str(), kv_id, node_sz);
  return id;
}

std::string Master::getInfoByInum(unsigned long long inum) {
  return register_id_to_addr_[inum_to_register_id_[inum]];
}

int Master::Move(std::string lock_server_addr, std::vector<unsigned long long> inum, std::string src, std::string dst) {
  for(auto id : inum) {
    printf("do move %016x from src %s to dst %s\n",id, src.c_str(), dst.c_str());
  }
  /* OK is always the first state. */
  int status = 0;

  auto pos = src.find_last_of(":");
  /* temp solution, rpc should be updated. */
  std::string src_ip = src.substr(0, pos);
  std::string src_port = src.substr(pos + 1);

  /* the same for lock_server */
  pos = lock_server_addr.find_last_of(":");
  std::string ls_ip = lock_server_addr.substr(0, pos);
  std::string ls_port = lock_server_addr.substr(pos + 1);

  /* Create a rpc client to connect the src extent_server. */
  sockaddr_in src_sin;
  make_sockaddr(src_port.c_str(), &src_sin);
  std::unique_ptr<rpcc> ptr_rpc_cl(new rpcc(src_sin));
  if (ptr_rpc_cl->bind() != 0) {
    printf("Master/move failed: failed to bind the rpc client.\n");
    return extent_protocol::RPCERR;
  }

  /* create a rpc client for lock server. */
  std::unique_ptr<lock_client> lc(new lock_client_cache(ls_port, nullptr));

  /* lock all the files. */
  lock_protocol::status s_lock = lock_protocol::OK;
  bool go_on = true;
  for (int i = 0; i < (int)inum.size(); ++i) {
    s_lock = lc->acquire(inum[i]);
    if (s_lock != lock_protocol::OK) {
      go_on = false;
      status = s_lock;

      /* release all locked file. */
      for (int j = i - 1; j >= 0; --j) {
        lc->release(inum[j]);
      }
    }
  }
  
  /* ERROR occurred when lock files to be moved. */ 
  if (!go_on) {
    printf("error: lock file error in master!\n");
    return status;
  }

  extent_protocol::status ret = extent_protocol::OK; /* status for rpc call */
  int r; /* status for move func */
  ret = ptr_rpc_cl->call(extent_protocol::move, std::move(inum), std::move(dst), r);
  if (ret != extent_protocol::OK) {
    printf("error: move operation called by master!\n");
  }

  /* release all the lock. */
  for (auto i : inum) {
    lc->release(i);
  }

  return ret;
}

int Master::calculateScore(std::vector<std::shared_ptr<AdjustContext> >* context_vec) {
  unsigned long long sum;
  unsigned long long cur_sum;
  for(auto context : (*context_vec)) {
    cur_sum += context->cur_memory;
    sum += context->info->sum_memory;
  }
  return (1.0 * cur_sum) / (sum);
}

}  // namespace master
}  // namespace ctgfs

