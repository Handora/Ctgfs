/*
* author: OneDay_(ltang970618@gmail.com)
**/

#include <client/client.h>
#include <parser/parser.h>
#include <algorithm>
#include <iostream>
#include <string>

namespace ctgfs {
namespace client {
Client::Client() {}

Client::Client(const std::string& input) { Client(input, DEFAULT_SERVER_ADDR); }

Client::Client(const std::string& input, const std::string& ip, const int port)
    : command_input_(input) {
  initChannel(ip, port);
}

Client::Client(const std::string& input, const std::string& addr)
    : command_input_(input) {
  initChannel(addr);
}

bool Client::StartClient() { return parserInput(); }

void Client::initChannel(const std::string& addr) {
  auto split_pos =
      std::find_if(addr.begin(), addr.end(), [](char x) { return (x == ':'); });
  std::string ip = std::string(addr.begin(), split_pos);
  std::string port_str = std::string(split_pos + 1, addr.end());
  int port = atoi(port_str.c_str());
  initChannel(ip, port);
}

void Client::initChannel(const std::string& ip, const int port) {
  client_response_ptr_ = std::make_shared<ClientKVResponse>();
  brpc::ChannelOptions options;
  client_channel_.Init(ip.c_str(), port, &options);
}

bool Client::parserInput() {
  client_request_ptr_ = std::make_shared<ClientKVRequest>();
  parser::Parser parser;
  if (!parser.ParseFromInput(command_input_, (*client_request_ptr_.get()))) {
    debugErrorParserInput(true, "Parse Input Command Error!");
    return false;
  }
  debugErrorParserInput(false,
                        "Parse Successfully, Starting Connecting Master");
  return connectToMaster();
}

bool Client::connectToMaster() {
  debugErrorConnectToMaster(
      false, "Master Connect Successfully\nStarting Communicating");
  return askKV();
}

bool Client::askKV() {
  MasterService_Stub stub(&client_channel_);
  brpc::Controller ctrl;
  stub.ClientAskForKV(&ctrl, client_request_ptr_.get(),
                      client_response_ptr_.get(), NULL);
  if (ctrl.Failed()) {
    debugErrorAskKV(true, "Fail to get target address!");
    return false;
  } else {
    debugErrorAskKV(false, "Successfully!Start connecting target!");
    return connectToKV();
  }
}

bool Client::connectCallback() {}

bool Client::connectToKV() {
  // connect kv
}

bool Client::doCommand() {
  // do command
}

void Client::debugErrorParserInput(bool is_error, const char* str) {
  debugErrorParserInput(is_error, std::string(str));
}

void Client::debugErrorParserInput(bool is_error,
                                   const std::string& error_str) {
  if (is_error) {
    // std::cout << error_str << std::endl;
  } else {
    // std::cout << error_str << std::endl;
  }
}

void Client::debugErrorConnectToMaster(bool is_error, const char* str) {
  debugErrorConnectToMaster(is_error, std::string(str));
}

void Client::debugErrorConnectToMaster(bool is_error,
                                       const std::string& error_str) {
  if (is_error) {
    // std::cout << error_str << std::endl;
  } else {
    // std::cout << error_str << std::endl;
  }
}

void Client::debugErrorAskKV(bool is_error, const char* str) {
  debugErrorAskKV(is_error, std::string(str));
}

void Client::debugErrorAskKV(bool is_error, const std::string& error_str) {
  if (is_error) {
    // std::cout << error_str << std::endl;
  } else {
    // std::cout << error_str << std::endl;
  }
}
}
}
