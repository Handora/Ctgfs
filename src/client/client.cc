#include <client/client.h>
#include <iostream>
#include <parser/parser.h>
#include <string>
#include <algorithm>

namespace ctgfs{
namespace client{
	Client::Client(){}
	Client::Client(const std::string& input) {
		Client(input,DEFAULT_SERVER_ADDR);
	}

	Client::Client(const std::string& input,const std::string& ip, const int port)
 		: command_input_(input)	{
			InitChannel(ip,port);
	}

	Client::Client(const std::string& input,const std::string& addr)
		: command_input_(input) {
			InitChannel(addr);
	}

	bool Client::StartClient() {
		return ParserInput();	
	}

	void Client::InitChannel(const std::string& addr) {
		auto split_pos = std::find_if(addr.begin(),addr.end(),
			[](char x) { 
				return (x == ':');
			}
		);	
		std::string ip = std::string(addr.begin(),split_pos);
		std::string port_str = std::string(split_pos + 1,addr.end());
		int port = atoi(port_str.c_str());
		InitChannel(ip,port);
	}

	void Client::InitChannel(const std::string& ip, const int port) {
		client_response_ptr_ = std::make_shared<ClientKVResponse>();
		brpc::ChannelOptions options;
		client_channel_.Init(ip.c_str(),port,&options);
	}

	bool Client::ParserInput() {
		client_request_ptr_ = std::make_shared<ClientKVRequest>();
		parser::Parser parser;
		if(!parser.ParseFromInput(command_input_,(*client_request_ptr_.get()))) {
			DebugErrorParserInput(true,"Parse Input Command Error!");
			return false;
		}
		DebugErrorParserInput(false,"Parse Successfully, Starting Connecting Master");
		return ConnectToMaster();
	}

	bool Client::ConnectToMaster() {
		DebugErrorConnectToMaster(false, "Master Connect Successfully\nStarting Communicating");
		return AskKV();
	}

	bool Client::AskKV() {
		MasterService_Stub stub(&client_channel_);
		brpc::Controller ctrl;
		stub.ClientAskForKV(&ctrl, client_request_ptr_.get(), 
				client_response_ptr_.get(), NULL);
		if(ctrl.Failed()) {
			DebugErrorAskKV(true,"Fail to get target address!");	
			return false;	
		}
		else {
			DebugErrorAskKV(false,"Successfully!Start connecting target!");
			return ConnectToKV();
		}
	}

	bool Client::ConnectCallback() {
	
	}

	bool Client::ConnectToKV() {
		// connect kv
	}

	bool Client::DoCommand() {
		// do command
	}

	void Client::DebugErrorParserInput(bool is_error,const char* str) {
		DebugErrorParserInput(is_error,std::string(str));
	}

	void Client::DebugErrorParserInput(bool is_error,const std::string& error_str) {
		if(is_error) {
			std::cout << error_str << std::endl;
		}
		else {
			std::cout << error_str << std::endl;
		}
	}

	void Client::DebugErrorConnectToMaster(bool is_error,const char* str) {
		DebugErrorConnectToMaster(is_error,std::string(str));
	}

	void Client::DebugErrorConnectToMaster(bool is_error,const std::string& error_str) {
		if(is_error) {
			std::cout << error_str << std::endl;
		}
		else {
			std::cout << error_str << std::endl;
		}
	}

	void Client::DebugErrorAskKV(bool is_error,const char* str) {
		DebugErrorAskKV(is_error,std::string(str));
	}
	
	void Client::DebugErrorAskKV(bool is_error,const std::string& error_str) {
		if(is_error) {
			std::cout << error_str << std::endl;
		}
		else {
			std::cout << error_str << std::endl;
		}
	}
}
}
