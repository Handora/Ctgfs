/*
 * author: OneDay_(ltang970618@gmail.com)
 **/
#pragma once
#include <brpc/channel.h>
#include <butil/time.h>
#include <client.pb.h>
#include <gflags/gflags.h>
#include <memory>
#include <string>
#include <util/status.h>

namespace ctgfs {
namespace client {
// default server addr
const std::string DEFAULT_SERVER_ADDR = "127.0.0.1:1234";
class Client {
 public:
  // the string of command as input ip:prot as addr
  // Example:
  // Client clt = Client("rm /src");
  // Client clt = Client("rm /src", "127.0.0.1", 1234);
  // Client clt = Client("rm /src", "127.0.0.1:1234");
  Client(const std::string& input);
  Client(const std::string& input, const std::string& ip, const int port);
  Client(const std::string& input, const std::string& addr);
  // start a client end when transfer successfully
  // return true if successfully
  // return false when any time error happens
  bool StartClient();

 private:
  // should not use default constructor
  Client()=delete;
  // should be init every time before connect
  brpc::Channel client_channel_;
  // store the file string
  std::string command_value_;
  std::shared_ptr<ClientKVRequest> client_request_ptr_;
  std::shared_ptr<ClientKVResponse> client_response_ptr_;
  const std::string command_input_;
  // will init client_channel_ and client_response_ptr_
  // so be careful because you will lose the info of clientKVresp
  void initChannel(const std::string& addr);
  void initChannel(const std::string& ip, const int port);
  // return true when parse succ else return false
  // this fuction will remove the value of command store in client if value exists
  // prevent transfering value when rpc
  util::Status parserInput();
  // return true when connect succ eles return false
  util::Status connectToMaster();
  // to get kv addr
  util::Status askKV();
  // wait to imporve
  util::Status connectCallback();
  util::Status connectToKV();
  // convenient to change to multithread
  util::Status doCommand();
  // 
  util::Status doCommandWithStream(); 
  // for debug and log
  // if error is_error = true else false to help decide the level of log
  void debugErrorParserInput(bool is_error, const char*);
  void debugErrorParserInput(bool is_error, const std::string& error_str = "");
  void debugErrorConnectToMaster(bool, const char*);
  void debugErrorConnectToMaster(bool, const std::string& error_str = "");
  void debugErrorAskKV(bool, const char*);
  void debugErrorAskKV(bool, const std::string&);
};
}  // namespace client
}  // namespace ctgfs
