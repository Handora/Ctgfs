/*
 * author: OneDay_(ltang970618@gmail.com)
 **/

#include "client/client.h"
#include <sys/time.h>
#include <time.h>
#include <algorithm>
#include <iostream>
#include <string>

namespace ctgfs {
namespace client {

Client::~Client() {
}

Client::Client(const std::string& ip, const int& port) {
  master_addr_ = ip + ":" + std::to_string(port);
}

Client::Client(const std::string& addr) : master_addr_(addr) {}

std::string Client::GetKVAddrByInum(Client::inum ino) {
  auto ele = getKVAddrByInumViaCache(ino);
  if(ele == "") {
    ele = getKVAddrByInumViaMaster(ino);
    if(ele != "") {
      linkToKVAddr(ino, ele);
    }
  }
  return ele;
}

void Client::FailCache(inum ino) {
  kv_addr_connect_status_[ino] = 1;
  auto id = kv_addr_index_list_[ino];
  auto& ele = kv_addr_list_[id];
  ele.second --;
  if(ele.second == 0) {
    ele.first = "";
    ele.second = - 1;
    reused_queue_.push(id);
  }
}

std::string Client::getKVAddrByInumViaCache(Client::inum ino) {
  if(kv_addr_connect_status_.find(ino) != kv_addr_connect_status_.end() && kv_addr_connect_status_[ino]) {
    return kv_addr_list_[kv_addr_index_list_[ino]].first;
  }
  else {
    return "";
  }
}

std::string Client::getKVAddrByInumViaMaster(inum ino) {
  std::string r = "";
  auto cl = getRpc(master_addr_);
  if(!cl) {
    return r;
  }
  cl->call(master_protocol::ask_for_kv, ino, r);
  return r;
}

std::pair<Client::inum, std::string> Client::GetInumByName(const std::string& name, bool is_dir, unsigned long long sz) {
  sockaddr_in ms_sin;
  make_sockaddr(master_addr_.c_str(), &ms_sin);
  auto cl = getRpc(master_addr_);
  std::string addr = "";
  std::pair<unsigned long long, std::string> r = std::make_pair(0, addr); 
  if(!cl) {
    return r;
  }
  cl->call(master_protocol::ask_for_ino, name, is_dir, sz, r);
  return r;
}

int Client::getNextId() {
  if(reused_queue_.empty()) {
    kv_addr_list_.push_back(std::make_pair("", 0));
    return (int)kv_addr_list_.size() - 1;
  }
  else {
    auto id = reused_queue_.front();
    reused_queue_.pop();
    return id;
  }
}

void Client::linkToKVAddr(inum ino, const std::string& addr) {
  for(int i = 0;i != (int)kv_addr_index_list_.size(); i ++) {
    auto& ele = kv_addr_list_[i];
    if(ele.first == addr && ele.second != -1) {
      ele.second ++;
      kv_addr_index_list_[ino] = i;
      kv_addr_connect_status_[i] = 0;
      return;
    }
  }
  auto id = getNextId();
  auto& ele = kv_addr_list_[id];
  kv_addr_connect_status_[id] = 0;
  ele.first = addr;
  ele.second ++;
  kv_addr_index_list_[ino] = id;
}

std::shared_ptr<rpcc> Client::getRpc(const std::string& addr) {
  if(rpc_pool_.find(addr) != rpc_pool_.end()) {
    return rpc_pool_[addr];
  }
  sockaddr_in ms_sin;
  make_sockaddr(master_addr_.c_str(), &ms_sin);
  std::shared_ptr<rpcc> cl = std::make_shared<rpcc>(ms_sin);
  if(cl->bind() != 0) {
    return nullptr;
  }
  rpc_pool_[addr] = cl;
  return cl;
}

}  // namespace client
}  // namespace ctgfs


