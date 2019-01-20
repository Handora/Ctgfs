/*
 * author: OneDay_(ltang970618@gmail.com)
 **/

#include <brpc/stream.h>
#include <butil/logging.h>
#include <client/client.h>
#include <parser/parser.h>
#include <sys/time.h>
#include <time.h>
#include <algorithm>
#include <iostream>
#include <string>

namespace ctgfs {
namespace client {

Client::Client() { Client(DEFAULT_SERVER_ADDR); }

Client::~Client() {
  {
    std::unique_lock<std::mutex> lock_guard(task_queue_mutex_);
    is_stop_ = true;
  }
  thread_condition_.notify_all();
  for (auto& running_thread : thread_vec_) running_thread.join();
}

Client::Client(const std::string& ip, const int& port) {
  master_addr_ = ip + ":" + std::to_string(port);
}

Client::Client(const std::string& addr) : master_addr_(addr) {}

util::Status Client::AddTask(const std::string& command_input) {
  std::shared_ptr<ClientKVRequest> req_ptr =
      std::make_shared<ClientKVRequest>();
  auto parser_status = parserInput(command_input, req_ptr);
  if (parser_status.IsOK()) {
    auto task_ptr = std::make_shared<ClientTask>(req_ptr);
    {
      std::unique_lock<std::mutex> lock_guard(task_queue_mutex_);
      if (is_stop_) {
        return util::Status::ClientStop();
      }
      task_queue_.emplace(task_ptr);
    }
  }
  return parser_status;
}

util::Status Client::StartClient(int thread_num) {
  thread_num = std::max(1, thread_num);
  for (int i = 0; i < thread_num; i++) {
    thread_vec_.emplace_back([&]() {
      while (true) {
        std::shared_ptr<ClientTask> task_ptr = nullptr;
        {
          std::unique_lock<std::mutex> lock_guard(task_queue_mutex_);
          while (!is_stop_ && task_queue_.empty()) {
            thread_condition_.wait(lock_guard);
          }
          if (is_stop_ && task_queue_.empty()) {
            return;
          }
          task_ptr = std::move(task_queue_.front());
          task_queue_.pop();
        }
        if (task_ptr != nullptr) connectToMaster(task_ptr);
      }
    });
  }
  return util::Status::OK();
}

util::Status Client::parserInput(const std::string& command_input,
                                 std::shared_ptr<ClientKVRequest> req_ptr) {
  util::Status s = parser_.ParseFromInput(command_input, (*req_ptr.get()));
  if (!s.IsOK()) {
    debugErrorParserInput(true, "Parse Input Command Error!");
    return util::Status::ParserInputError();
  }
  return util::Status::OK();
}

util::Status Client::connectToMaster(std::shared_ptr<ClientTask> task_ptr) {
  // debugErrorConnectToMaster(
  //     false, "Master Connect Successfully\nStarting Communicating");
  return askKV(task_ptr);
}

util::Status Client::askKV(std::shared_ptr<ClientTask> task_ptr) {
  brpc::Channel channel;
  brpc::ChannelOptions options;
  channel.Init(master_addr_.c_str(), &options);
  MasterService_Stub stub(&channel);
  brpc::Controller ctrl;
  auto client_request_ptr = task_ptr->client_request_ptr;
  auto client_response_ptr = task_ptr->client_response_ptr;
  stub.ClientAskForKV(&ctrl, client_request_ptr.get(),
                      client_response_ptr.get(), NULL);
  if (ctrl.Failed()) {
    LOG(ERROR) << ctrl.ErrorText() << std::endl;
    return util::Status::NotFound("Fail to get target address");
  } else {
    return connectToKV(task_ptr);
  }
}

util::Status Client::connectCallback() { return util::Status::OK(); }

util::Status Client::connectToKV(std::shared_ptr<ClientTask> task_ptr) {
  // connect kv
  return doCommand(task_ptr);
  // return true;
}

util::Status Client::doCommand(std::shared_ptr<ClientTask> task_ptr) {
  // do command
  if (!(task_ptr->command_value).empty()) {
    return doCommandWithStream(task_ptr);
  }
  const std::string& fs_addr = task_ptr->client_response_ptr->addr();
  brpc::ChannelOptions options;
  brpc::Channel channel;
  channel.Init(fs_addr.c_str(), &options);
  brpc::Controller ctrl;
  FileSystemService_Stub stub(&channel);
  auto fs_res_ptr = std::make_shared<FileSystemResponse>();
  stub.DoCommandOnFS(&ctrl, (task_ptr->client_request_ptr).get(),
                     (fs_res_ptr).get(), NULL);
  if (ctrl.Failed()) {
    LOG(ERROR) << "Connect Fail" << std::endl;
    return util::Status::ConnectFailed();
  }
  return util::Status::OK();
}

util::Status Client::doCommandWithStream(std::shared_ptr<ClientTask> task_ptr) {
  brpc::Controller ctrl;
  brpc::StreamId stream;
  if (brpc::StreamCreate(&stream, ctrl, NULL) != 0) {
    LOG(ERROR) << "Fail to create stream" << std::endl;
    return util::Status::StreamCreateFailed();
  }
  brpc::ScopedStream stream_guard(stream);

  const std::string& fs_addr = task_ptr->client_response_ptr->addr();
  brpc::ChannelOptions options;
  brpc::Channel channel;
  channel.Init(fs_addr.c_str(), &options);
  FileSystemService_Stub stub(&channel);
  auto fs_res_ptr = task_ptr->file_system_response_ptr;
  stub.DoCommandOnFS(&ctrl, (task_ptr->client_request_ptr).get(),
                     fs_res_ptr.get(), NULL);
  auto command_value = task_ptr->command_value;
  int st = 0, ed = std::min(static_cast<int>(command_value.size()), 1023);
  do {
    std::string buf_str =
        std::string(command_value.begin() + st, command_value.begin() + ed);
    butil::IOBuf buf;
    buf.append(buf_str.c_str());
    int write_status = brpc::StreamWrite(stream, buf);
    if (write_status) {
      if (write_status == EAGAIN) {
        timespec t;
        t.tv_sec = 1;
        t.tv_nsec = 0;
        int flag = 0;
        do {
          flag = brpc::StreamWait(stream, &t);
          if (flag == EINVAL) {
            LOG(ERROR) << "Stream crash" << std::endl;
            return util::Status::StreamCrash();
          }
        } while (flag == ETIMEDOUT);
      } else if (write_status == EINVAL) {
        LOG(ERROR) << "Stream crash" << std::endl;
        return util::Status::StreamCrash();
      }
    } else {
      if (ed == static_cast<int>(command_value.size())) break;
      st = std::min(ed, static_cast<int>(command_value.size()));
      ed = std::min(ed + 1023, static_cast<int>(command_value.size()));
    }
  } while (ed <= static_cast<int>(command_value.size()));
  brpc::StreamClose(stream);
  return util::Status::OK();
}

void Client::debugErrorParserInput(bool is_error, const char* str) {
  debugErrorParserInput(is_error, std::string(str));
}

void Client::debugErrorParserInput(bool is_error,
                                   const std::string& error_str) {
  if (is_error) {
    LOG(ERROR) << error_str << std::endl;
    // std::cout << error_str << std::endl;
  } else {
    LOG(INFO) << error_str << std::endl;
    // std::cout << error_str << std::endl;
  }
}

void Client::debugErrorConnectToMaster(bool is_error, const char* str) {
  debugErrorConnectToMaster(is_error, std::string(str));
}

void Client::debugErrorConnectToMaster(bool is_error,
                                       const std::string& error_str) {
  if (is_error) {
    LOG(ERROR) << error_str << std::endl;
    // std::cout << error_str << std::endl;
  } else {
    LOG(INFO) << error_str << std::endl;
    // std::cout << error_str << std::endl;
  }
}

void Client::debugErrorAskKV(bool is_error, const char* str) {
  debugErrorAskKV(is_error, std::string(str));
}

void Client::debugErrorAskKV(bool is_error, const std::string& error_str) {
  if (is_error) {
    LOG(ERROR) << error_str << std::endl;
    // std::cout << error_str << std::endl;
  } else {
    LOG(INFO) << error_str << std::endl;
    // std::cout << error_str << std::endl;
  }
}
}  // namespace client
}  // namespace ctgfs
