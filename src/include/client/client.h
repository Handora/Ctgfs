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
		void InitChannel(const std::string& addr);
		void InitChannel(const std::string& ip, const int port);
		bool ParserInput();
		bool ConnectToMaster();
		bool AskKV();
		// wait to imporve
		bool ConnectCallback();	
		bool ConnectToKV();
		bool DoCommand();
		void DebugErrorParserInput(bool is_error,const char*);
		void DebugErrorParserInput(bool is_error,const std::string& error_str = "");
		void DebugErrorConnectToMaster(bool,const char*);
		void DebugErrorConnectToMaster(bool,const std::string& error_str = "");
		void DebugErrorAskKV(bool, const char*);
		void DebugErrorAskKV(bool, const std::string&);
	};
}
}
