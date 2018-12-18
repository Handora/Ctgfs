#pragma once
#include <parser/command.h>
#include <client.pb.h>
#include <memory>

namespace ctgfs {
namespace client {
	std::string DEFAULT_SERVER_ADDR = "127.0.0.1:1234";
	class Client {
	 public: 
		Client();
		Client(const std::string& input);
		Client(const std::string& input,const std::string& addr);
		bool StartClient();
	 private:
		std::shared_ptr<Command> command_ptr_;
		const std::string server_addr_;
		const std::string command_input_;
		bool ParserInput();
		bool ConnectToMaster();
		bool AskKV();
		bool ConnectToKV();
		bool DoCommand();
		void DebugErrorParserInput(bool is_error,const std::string& error_str = "");
		void DebugErrorConnectToMaster(bool,const std::string& error_str = "");
	}
}
}
