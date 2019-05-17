/*
 * author: OneDay_(ltang970618@gmail.com)
 **/
#pragma once
#include <master/prefix_tree.h>
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <vector>
#include <thread>
#include "fs/info_collector.h"
#include <mutex>

// as a server
// several part :
// 1. accept kv's info to solve control
// 2. accept client's req
// (TODO) 3. determin the kv according to the kv's info and response client
// (TODO) 4. add a struct to put kv's Info in it
//           so that master can do a choice by Info
// (TODO) 5. add service for kv to connect and disconnect
// (TODO) 6. add redis to help kv write file
namespace ctgfs {
namespace master {
using namespace info_collector;
using namespace ::ctgfs::prefix_tree;
class Master {
 public:
  // src kv id, dst kv id
  typedef std::pair<int, int> kv_info_t;
  Master();
  ~Master();
  int AskForIno(std::string path, bool is_dir, unsigned long long node_sz, std::pair<unsigned long long, std::string>& r);
  int AskForKV(unsigned long long ino, std::string&);

  /* move is used to achieve load balancing. */
  /* @inum the vector of some inum, files to be moved. */
  /* @src the ip:port of source extent_server. */
  /* @dst the ip:port of target extent_server. */
  int Move(std::string lock_server_addr, std::vector<unsigned long long> inum, std::string src, std::string dst);
  int UpdateKVInfo(InfoCollector::ServerInfo i, int&);
  int Regist(std::string, unsigned long long, int&);
 private:
  // ino count mutex
  std::mutex ino_count_mutex_;
  // file ino count
  unsigned long long ino_count_ = 0;
  // char set need hash
  std::string VALID_CHAR_SET;
  // lock of regist id
  std::mutex regist_lock_;
  // master is stop
  bool stop_ = false;
  // adjust thread
  std::thread adjust_thread_;
  // record the kv's id on master and kv's addr
  // int -> id
  // addr -> ip:port
  // Example :
  // <1, "127.0.0.1:1234">
  // get addr by register id
  std::map<int, std::string> register_id_to_addr_;
  // <path, register_id>
  std::map<int, int> inum_to_register_id_;
  std::map<int, std::string> inum_to_path_;
  // get register id by addr
  std::map<std::string, int> addr_to_register_id_;
  // std::vector<std::shared_ptr<heart_beat::HeartBeatInfo> > kv_info_;
  // every kv connect to master will get a distinct id
  int cur_register_kv_id_ = 0;
  // collect disconnect kv's id
  // to make hash space closer
  std::queue<int> reused_queue_;
  // prefix tree
  std::shared_ptr<PrefixTree> t_;
  // called when kv info update
  // void updateKVInfo(const std::shared_ptr<ctgfs::heart_beat::HeartBeatInfo>);
  // fill the info of resp
  // PROBLEM: if a kv disconnect it's file will lose
  // should have a dynamic method to track a kv's connect and disconnect
  // can't simply hash to a continuous register id
  // void setKVAddrByClientKVRequest(const ::ctgfs::ClientKVRequest* request,
  //                                ::ctgfs::ClientKVResponse* response);
  // register a new kv
  // if success return true else false
  // addr : ip:port
  bool registerKV(const std::string& ip, const int& port);
  bool registerKV(const std::string& addr);
  // get new kv
  // if register fail return -1
  // else return a distinc regist id
  int getNewRegisterID();
  // use kv's addr and register_id regist to the map
  void doRegister(const std::string&, const int&);
  // simple hash wait to improve
  // path -> a distinct regist id
  // you should guarantee the path is valid
  int hashPath(const std::string& path);
  // hash a string
  int hashStr(const std::string&);
  // solve a value to [0,cur_register_kv_id_)
  void hashValueToRegisterID(int&);
  // use regist id to unregister
  void unregisterKV(const int& regist_id);
  // do unregist
  void doUnregister(const int& regist_id);
  // generate inum
  unsigned long long genInum(const std::string& path, bool is_dir, unsigned long long node_sz);
  // get kv info by inum
  std::string getInfoByInum(unsigned long long inum);
  // calculate the score of current situation
  int calculateScore(std::vector<std::shared_ptr<AdjustContext> >* context_vec);
  // for debug
  void debugRegisterKV(bool, const std::string&);
  void debugRegisterKV(bool, const char*);
};
}  // namespace master
}  // namespace ctgfs
