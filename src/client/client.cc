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

Client::Client(const std::string& input) { Client(input, DEFAULT_SERVER_ADDR); }

Client::Client(const std::string& input, const std::string& ip, const int port)
    : command_input_(input) {
  initChannel(ip, port);
}

Client::Client(const std::string& input, const std::string& addr)
    : command_input_(input) {
  initChannel(addr);
}

bool Client::StartClient() {
  auto res = parserInput();
  return res.IsOK();
}

void Client::initChannel(const std::string& addr) {
  client_response_ptr_ = std::make_shared<ClientKVResponse>();
  brpc::ChannelOptions options;
  client_channel_.Init(addr.c_str(), &options);
}

void Client::initChannel(const std::string& ip, const int port) {
  client_response_ptr_ = std::make_shared<ClientKVResponse>();
  brpc::ChannelOptions options;
  client_channel_.Init(ip.c_str(), port, &options);
}

util::Status Client::parserInput() {
  client_request_ptr_ = std::make_shared<ClientKVRequest>();
  parser::Parser parser;
  util::Status s =
      parser.ParseFromInput(command_input_, (*client_request_ptr_.get()));
  if (!s.IsOK()) {
    debugErrorParserInput(true, "Parse Input Command Error!");
    return util::Status::ParserInputError();
  }
  if (client_request_ptr_->has_command()) {
    auto command = client_request_ptr_->command();
    if (command.has_value()) {
      command_value_ = std::move(command.value());
      command.clear_value();
    }
  }
  debugErrorParserInput(false,
                        "Parse Successfully, Starting Connecting Master");
  return connectToMaster();
}

util::Status Client::connectToMaster() {
  debugErrorConnectToMaster(
      false, "Master Connect Successfully\nStarting Communicating");
  return askKV();
}

util::Status Client::askKV() {
  MasterService_Stub stub(&client_channel_);
  brpc::Controller ctrl;
  stub.ClientAskForKV(&ctrl, client_request_ptr_.get(),
                      client_response_ptr_.get(), NULL);
  if (ctrl.Failed()) {
    LOG(ERROR) << "Fail to get target address" << std::endl;
    return util::Status::NotFound("Fail to get target address");
  } else {
    return connectToKV();
  }
}

util::Status Client::connectCallback() { return util::Status::OK(); }

util::Status Client::connectToKV() {
  // connect kv
  const std::string& kv_addr = client_response_ptr_->addr();
  initChannel(kv_addr);
  // TODO(multithread)
  return doCommand();
  // return true;
}

util::Status Client::doCommand() {
  // do command
  const std::string& fs_addr = client_response_ptr_->addr();
  initChannel(fs_addr);
  if (!command_value_.empty()) {
    return doCommandWithStream();
  }
  brpc::Controller ctrl;
  FileSystemService_Stub stub(&client_channel_);
  auto fs_res_ptr = std::make_shared<FileSystemResponse>();
  stub.DoCommandOnFS(&ctrl, client_request_ptr_.get(), fs_res_ptr.get(), NULL);
  if (ctrl.Failed()) {
    LOG(ERROR) << "Connect Fail" << std::endl;
    return util::Status::ConnectFailed();
  }
  return util::Status::OK();
}

util::Status Client::doCommandWithStream() {
  brpc::Controller ctrl;
  brpc::StreamId stream;
  if (brpc::StreamCreate(&stream, ctrl, NULL) != 0) {
    LOG(ERROR) << "Fail to create stream" << std::endl;
    return util::Status::StreamCreateFailed();
  }
  brpc::ScopedStream stream_guard(stream);
  int st = 0, ed = std::min((int)command_value_.size(), 1023);
  do {
    std::string buf_str =
        std::string(command_value_.begin() + st, command_value_.begin() + ed);
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
      st = std::min(ed, (int)command_value_.size());
      ed = std::min(ed + 1023, (int)command_value_.size());
    }
  } while (ed < (int)command_value_.size());
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
