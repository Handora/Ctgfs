/*
* author: OneDay_(ltang970618@gmail.com)
**/

#include <brpc/server.h>
#include <butil/logging.h>
#include <master.pb.h>
#include <client/client.h>
#include <gflags/gflags.h>
#include <gtest/gtest.h>
#include <fs/heart_beat_sender.h>
#include <fs/fs_service.h>
#include <master/master.h>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <iostream>

namespace ctgfs {
namespace master {
using namespace ctgfs::util;
using namespace ctgfs::client;
using namespace ctgfs::heart_beat;
using namespace ctgfs::fs;

// test [client] [master] [heart_beat_sender on kv] [file transfer]
// server start, heart_beat_detector start
// heart_beat_sender regist to the master
// client req
TEST(MasterTest, Connect) {
  // add service
  std::shared_ptr<Master> master_ptr = std::make_shared<Master>();
  brpc::Server server;
  int add_status =
      server.AddService(&(*master_ptr), brpc::SERVER_DOESNT_OWN_SERVICE);
  ASSERT_EQ(0, add_status) << "add server error! status : " << add_status
                           << std::endl;

  // start server
  brpc::ServerOptions options;
  std::string addr = std::string("127.0.0.1:1234");
  int start_status = server.Start(addr.c_str(), &options);
  ASSERT_EQ(0, start_status) << "start error! status : " << start_status
                             << std::endl;

  // new a heart_beat_sender
  std::string fs_addr = std::string("127.0.0.1:1233");
  auto heart_beat_info = std::make_shared<HeartBeatInfo>();
  heart_beat_info->type = HeartBeatMessageRequest_HeartBeatType::
      HeartBeatMessageRequest_HeartBeatType_kRegist;  // kRgist
  heart_beat_info->addr = fs_addr;
  HeartBeatSender sender(addr, heart_beat_info);
  auto send_status = sender.SendHeartBeat();
  EXPECT_EQ(send_status.IsOK(), true) << "heart beat fail" << std::endl;

  // add fs service
  std::string res_str;
  std::stringstream ss;
  std::function<void()> callback = [&]() {
    std::string cur_str;
    while(ss >> cur_str) {
      res_str += cur_str;
    }
  };
  std::shared_ptr<TestFSService> fs_service_ptr = 
    std::make_shared<TestFSService>(ss, callback);
  brpc::Server fs_server;
  add_status = fs_server.AddService(&(*fs_service_ptr), brpc::SERVER_DOESNT_OWN_SERVICE);
  ASSERT_EQ(0, add_status) << "add fs server error! status : " << add_status << std::endl;

  //start fs service
  brpc::ServerOptions fs_server_options;
  start_status = fs_server.Start(fs_addr.c_str(), &fs_server_options);
  ASSERT_EQ(0, start_status) << "start service error! status : " << start_status << std::endl;
  std::string test_str;
  for(int i = 0;i < 1024;i ++) {
    test_str += "qcnb";
  }
  auto client_func = [=]() {
    std::string input = "write /a/b " + test_str;
    Client client(addr);
    auto status_1 = client.StartClient();
    ASSERT_EQ(true, status_1.IsOK());
    auto status_2 = client.AddTask(input);
    EXPECT_EQ(true, status_2.IsOK());
  };
  std::thread t(client_func);
  t.join();
  fs_server.Stop(0);
  server.Stop(0);
  fs_server.Join();
  server.Join();
  EXPECT_STREQ(test_str.c_str(), res_str.c_str());
}

}  // namespace master
}  // namespace ctgfs
