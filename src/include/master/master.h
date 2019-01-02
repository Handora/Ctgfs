/*
 * author: OneDay_(ltang970618@gmail.com)
 **/
#pragma once
#include <brpc/channel.h>
#include <butil/time.h>
#include <client.pb.h>
#include <master/heart_beat.h>
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <vector>

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
class Master : public MasterService {
 public:
  Master();
  ~Master();
  void ClientAskForKV(::google::protobuf::RpcController* controller,
                      const ::ctgfs::ClientKVRequest* request,
                      ::ctgfs::ClientKVResponse* response,
                      ::google::protobuf::Closure* done);
  void UpdateKVInfo(const std::shared_ptr<ctgfs::heart_beat::HeartBeatInfo>);

 private:
  // char set need hash
  std::string VALID_CHAR_SET;
  // record the kv's id on master and kv's addr
  // int -> id
  // addr -> ip:port
  // Example :
  // <1, "127.0.0.1:1234">
  // get addr by register id
  std::map<int, std::string> register_id_to_addr_;
  // get register id by addr
  std::map<std::string, int> addr_to_register_id_;
  std::vector<std::shared_ptr<heart_beat::HeartBeatInfo> > kv_info_;
  // every kv connect to master will get a distinct id
  int cur_register_kv_id_ = 0;
  // collect disconnect kv's id
  // to make hash space closer
  std::queue<int> reused_queue_;
  // fill the info of resp
  // PROBLEM: if a kv disconnect it's file will lose
  // should have a dynamic method to track a kv's connect and disconnect
  // can't simply hash to a continuous register id
  void setKVAddrByClientKVRequest(const ::ctgfs::ClientKVRequest* request,
                                  ::ctgfs::ClientKVResponse* response);
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
  // for debug
  void debugRegisterKV(bool, const std::string&);
  void debugRegisterKV(bool, const char*);
};
}  // namespace master
}  // namespace ctgfs
