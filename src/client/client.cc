#include <parser/client.h>

namespace ctgfs{
namespace client{
	Client::Client(){}
	Client::Client(const std::string& input) {
		Client(intput,DEFAULT_SERVER_ADDR);
	}
	Client::Client(const std::string& input,const std::string& addr)
		:server_addr_(addr), command_intput_(intput) {

	}

	bool Client::StartClient() {

	}

	bool Client::ParserInput() {
		command_ptr_ = std::make_shared<Command>();
		Parser parser;
		if(!parser.ParseFromInput(command_input_,command_ptr_.get())) {
			DebugErrorParserInput(true,"Parse Input Command Error!");
			return false;
		}
		DebugErrorParserInput(false,"Parse Successfully, Starting Connecting Master");
		ConnectToMaster();
	}

	bool Client::ConnectToMaster() {
		brpc::Channel channel;
		brpc::ChannelOptions options;
		channel.Init(server_addr_,&options);
	}

	bool Client::AskKV() {
		
	}

	bool Client::ConnectToKV() {
		
	}

	bool Client::DoCommand() {

	}

	void Client::DebugErrorParserInput(bool is_error,const std::string& error_str) {
		if(is_error) {
			std::cout << error_str << std::endl;
		}
		else {
			std::cout << error_str << std::endl;
		}
	}

	void Client::DebugErrorConnectToMaster(bool is_error,const std::string& error_str) {
		if(is_error) {
			std::cout << error_str << std::endl;
		}
		else {
			std::cout << error_str << std::endl;
		}
	}
}
}
