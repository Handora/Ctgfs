/*
 * author: OneDay_(ltang970618@gmail.com)
 **/
#pragma once
#include <brpc/channel.h>
#include <butil/time.h>
#include <fs.pb.h>
#include <gflags/gflags.h>
#include <master.pb.h>
#include <parser/parser.h>
#include <util/status.h>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace ctgfs {
namespace client {
// default server addr
const std::string DEFAULT_SERVER_ADDR = "127.0.0.1:1234";
// a client command is a task
// this struct is to wrap the info of a client task
struct ClientTask {
  ClientTask(std::shared_ptr<ClientKVRequest> client_req_ptr)
      : client_request_ptr(client_req_ptr) {
    // prevent transfer file stream
    // store locally
    if (client_request_ptr->has_command()) {
      auto command = client_request_ptr->command();
      if (command.has_value()) {
        command_value = std::move(command.value());
        command.clear_value();
      }
    }
    client_response_ptr = std::make_shared<ClientKVResponse>();
    file_system_response_ptr = std::make_shared<FileSystemResponse>();
  }
  std::shared_ptr<ClientKVRequest> client_request_ptr;
  std::shared_ptr<ClientKVResponse> client_response_ptr;
  std::shared_ptr<FileSystemResponse> file_system_response_ptr;
  std::string command_value;
};

// every task manage own resource(req/resp channel controller ...)
// this class just help choose task to do command and help find master's addr
// but not store info
// you should use master's addr or ip:prot to construct a client
class Client {
 public:
  Client();  // use default addr
  Client(const std::string& ip, const int& port);
  // addr(ip:port)
  // Example:
  // 127.0.0.1:1234
  Client(const std::string& addr);
  // should join all thread
  ~Client();
  // construct a client you should call AddTask to start a command
  // the string of command as input ip:prot as addr
  // add a command task
  // return true if command successfully
  // return false whenever error happens
  // Example:
  // ("rm /src");
  util::Status AddTask(const std::string& input);
  // start [thread_num] thread to do command
  // you should call this function
  // this function will start select tasks from task queue
  util::Status StartClient(int thread_num = 1);

 private:
  // stop flag if true thread can't get task
  bool is_stop_ = false;
  // condition of thread
  std::condition_variable thread_condition_;
  // mutex of task queue
  std::mutex task_queue_mutex_;
  // task queue
  std::queue<std::shared_ptr<ClientTask> > task_queue_;
  // store master addr
  // ip:port
  std::string master_addr_;
  // parser
  parser::Parser parser_;
  // store thread
  std::vector<std::thread> thread_vec_;
  // this is to fill a ClientKVRequest by input info
  util::Status parserInput(const std::string& command_input,
                           std::shared_ptr<ClientKVRequest> req);
  // return true when connect succ eles return false
  util::Status connectToMaster(std::shared_ptr<ClientTask> task_ptr);
  // to get kv addr
  util::Status askKV(std::shared_ptr<ClientTask> task_ptr);
  // wait to imporve
  util::Status connectCallback();
  util::Status connectToKV(std::shared_ptr<ClientTask> task_ptr);
  // convenient to change to multithread
  util::Status doCommand(std::shared_ptr<ClientTask> task_ptr);
  // create a stream to transfer file
  util::Status doCommandWithStream(std::shared_ptr<ClientTask> task_ptr);
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
