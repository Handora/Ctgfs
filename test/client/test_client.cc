/*
* author: fftlover(ltang970618@gmail.com)
**/

#include <brpc/server.h>
#include <butil/logging.h>
#include <client.pb.h>
#include <gflags/gflags.h>
#include <fs/fs_service.h>
#include <client/client.h>
#include <thread>
#include <functional>
#include <iostream>
#include <brpc/stream.h>
#include <gtest/gtest.h>

namespace ctgfs {
namespace client{

// test file transfer to fs with stream
TEST(ClientTest, Connect) {
  using namespace fs;
  // service prepare
  std::string test_str;
  for(int i = 0;i < 2048;i ++) {
    test_str.push_back('a');
  }
  std::string res_str;
  std::stringstream ss;
  std::function<void()> callback = [&](){
      std::string cur_str;
      while(ss >> cur_str) {
        res_str += cur_str;
      }
  };
  std::shared_ptr<TestFSService> fs_service_ptr
    = std::make_shared<TestFSService>(ss, callback) ;
  brpc::Server server;

  // add service
  int add_status = server.AddService(&(*fs_service_ptr), brpc::SERVER_DOESNT_OWN_SERVICE);
  ASSERT_EQ(0, add_status) << "add service error! status : " << add_status << std::endl;

  // start server
  brpc::ServerOptions options;
  std::string addr = std::string("127.0.0.1:1235");
  int start_status = server.Start(addr.c_str(), &options);
  ASSERT_EQ(0, start_status) << "start service error! status : " << start_status << std::endl;

  // init client info
  brpc::Channel channel;
  brpc::ChannelOptions channel_options;
  channel.Init(addr.c_str(), &channel_options);
  FileSystemService_Stub stub(&channel);
  auto fs_res_ptr = std::make_shared<FileSystemResponse>();
  auto req_ptr = std::make_shared<ClientKVRequest>();
  req_ptr->set_id(1);
  req_ptr->set_addr("127.0.0.1:1234");
  auto command_ptr = new ClientKVRequest_Command;
  command_ptr->set_type(ctgfs::ClientKVRequest_Command_Type_kWriteFile);
  command_ptr->set_path("/");
  req_ptr->set_allocated_command(command_ptr);
  // create stream
  brpc::Controller ctrl;
  brpc::StreamId stream;
  int stream_create_status = brpc::StreamCreate(&stream, ctrl, NULL);
  ASSERT_EQ(0, stream_create_status) << "create stream fail! status : " << stream_create_status << std::endl;
  brpc::ScopedStream stream_guard(stream);

  // shake hands
  stub.DoCommandOnFS(&ctrl, req_ptr.get(), fs_res_ptr.get(), NULL); 
  ASSERT_EQ(0, ctrl.Failed());

  // send stream
  int st = 0, ed = std::min((int)test_str.size(), 1023);
  do {
    std::string buf_str = std::string(test_str.begin() + st, test_str.begin() + ed);
    butil::IOBuf buf;
    buf.append(buf_str.c_str());
    int write_status = brpc::StreamWrite(stream, buf);
    ASSERT_NE(EINVAL, write_status) << "connect error" << std::endl;
    EXPECT_EQ(0, write_status) << "write stream fail" << std::endl;
    if(ed == (int)test_str.size())
      break;
    if(!write_status) {
      st = std::min(ed, (int)test_str.size());
      ed = std::min(ed + 1023, (int)test_str.size());
    }
  } while(ed <= (int)test_str.size());

  ASSERT_EQ(0, brpc::StreamClose(stream));
  server.Stop(0);
  server.Join();
  EXPECT_STREQ(test_str.c_str(), res_str.c_str());
}
} // namespace client
} // namespace ctgfs
