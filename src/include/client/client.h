#pragma once
#include <client.pb.h>
#include <memory>
#include <string>
#include <gflags/gflags.h>
#include <butil/time.h>
#include <brpc/channel.h>

namespace ctgfs {
namespace client {
	std::string DEFAULT_SERVER_ADDR = "127.0.0.1:1234";
  class Client {
   public: 
    Client();
    Client(const std::string& input);
    Client(const std::string& input,const std::string& ip,const int port);
    Client(const std::string& input,const std::string& addr);
    bool StartClient();
   private:
    brpc::Channel client_channel_;
    std::shared_ptr<ClientKVRequest> client_request_ptr_;
    std::shared_ptr<ClientKVResponse> client_response_ptr_;
    const std::string command_input_;
    void initChannel(const std::string& addr);
    void initChannel(const std::string& ip, const int port);
    bool parserInput();
    bool connectToMaster();
    bool askKV();
    // wait to imporve
    bool connectCallback();	
    bool connectToKV();
    bool doCommand();
    void debugErrorParserInput(bool is_error,const char*);
    void debugErrorParserInput(bool is_error,const std::string& error_str = "");
    void debugErrorConnectToMaster(bool,const char*);
    void debugErrorConnectToMaster(bool,const std::string& error_str = "");
    void debugErrorAskKV(bool, const char*);
    void debugErrorAskKV(bool, const std::string&);
  };
}
}
