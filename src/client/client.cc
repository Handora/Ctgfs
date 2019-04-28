/*
 * author: OneDay_(ltang970618@gmail.com)
 **/

#include <brpc/stream.h>
#include <butil/logging.h>
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
  brpc::Controller cntl;
  brpc::Channel channel;
  if(channel.Init(master_addr_.c_str(), &options) != 0) {
    LOG(ERROR) << "Fail to initialize channel" << std::endl;
    return "";
  }
  ::ctgfs::ClientAskForKVByInoRequest request;
  ::ctgfs::ClientAskForKVByInoResponse response;
  ::ctgfs::MasterService_Stub stub(&channel);
  request.set_ino(ino);
  stub.AskForKV(&cntl, &request, &response, NULL);
  if(!cntl.Failed()) {
    return response.addr();
  }
  return "";
}

std::pair<Client::inum, std::string> Client::GetInumByName(const std::string& name, bool is_dir) {
  brpc::Controller cntl;
  brpc::Channel channel;
  if(channel.Init(master_addr_.c_str(), &options) != 0) {
    LOG(ERROR) << "Fail to initialize channel" << std::endl;
    return std::make_pair(0, "");
  }
  ::ctgfs::ClientAskForInoRequest request;
  ::ctgfs::ClientAskForInoResponse response;
  ::ctgfs::MasterService_Stub stub(&channel);
  request.set_path(name);
  request.set_is_dir(is_dir);
  stub.AskForIno(&cntl, &request, &response, NULL);
  if(!cntl.Failed()) {
    printf("resp addr %s\n", response.addr().c_str());
    return std::make_pair(response.ino(), response.addr());
  }
  return std::make_pair(0, "");
}

}  // namespace client
}  // namespace ctgfs
