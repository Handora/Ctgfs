/*
 * author: OneDay_(ltang970618@gmail.com)
 **/
#pragma once
#include <master/master_protocol.h>
#include <rpc/marshall.h>
#include <rpc/rpc.h>
#include <util/status.h>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

using namespace ::ctgfs::rpc;

namespace ctgfs {
namespace client {

// default server addr
const std::string DEFAULT_SERVER_ADDR = "127.0.0.1:1234";
// TODO:add cache of kv's addr
// help to find file position of KV
// before every extend query
class Client {
 public:
  typedef unsigned long long inum;
  // Client();  // use default addr
  Client(const std::string& ip, const int& port);
  // addr(ip:port)
  // Example:
  // 127.0.0.1:1234
  Client(const std::string& addr = DEFAULT_SERVER_ADDR);
  // should join all thread
  ~Client();
  // use inum to find the KV
  std::string GetKVAddrByInum(inum ino);
  // send the path and get the inum generated by master
  std::pair<inum, std::string> GetInumByName(inum parent,
                                             const std::string& name,
                                             bool is_dir,
                                             unsigned long long sz);
  // fail cache
  void FailCache(inum ino);

 private:
  // store master addr
  // ip:port
  std::string master_addr_;
  // cache of kv addr
  // <ino, kv addr index>
  // kv addr index -> kv addr
  // if fail clear the kv_addr and retry
  std::map<inum, int> kv_addr_index_list_;
  // tag of connect status
  std::map<inum, int> kv_addr_connect_status_;
  // <addr, ref count>
  std::vector<std::pair<std::string, int> > kv_addr_list_;
  std::queue<int> reused_queue_;
  // <addr, rpcc>
  std::map<std::string, std::shared_ptr<rpcc> > rpc_pool_;
  int getNextId();
  void linkToKVAddr(inum ino, const std::string&);
  std::string getKVAddrByInumViaMaster(inum ino);
  std::string getKVAddrByInumViaCache(inum ino);
  std::shared_ptr<rpcc> getRpc(const std::string& addr);
};
}  // namespace client
}  // namespace ctgfs
