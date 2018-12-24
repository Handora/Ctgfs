/*
* author: OneDay_(ltang970618@gmail.com)
**/

#include <brpc/server.h>
#include <butil/logging.h>
#include <client.pb.h>
#include <client/client.h>
#include <gflags/gflags.h>
#include <gtest/gtest.h>
#include <master/heart_beat.h>
#include <master/heart_beat_detector.h>
#include <master/master.h>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

namespace ctgfs {
namespace master {
using namespace ctgfs::util;
using namespace ctgfs::client;
using namespace ctgfs::heart_beat;

// test [client] [master] [heart_beat_sender on kv] [heart_beat_detector on
// master]
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

  // add & start detector
  brpc::Server detector_server;
  HeartBeatDetector detector(master_ptr);
  add_status =
      detector_server.AddService(&detector, brpc::SERVER_DOESNT_OWN_SERVICE);
  ASSERT_EQ(0, add_status);
  brpc::ServerOptions detector_options;
  std::string detector_addr = std::string("127.0.0.1:1236");
  start_status =
      detector_server.Start(detector_addr.c_str(), &detector_options);
  ASSERT_EQ(0, start_status);

  // new a heart_beat_sender

  std::string heart_beat_sender_addr = std::string("127.0.0.1:1235");
  auto heart_beat_info = std::make_shared<HeartBeatInfo>();
  heart_beat_info->type = HeartBeatMessageRequest_HeartBeatType::
      HeartBeatMessageRequest_HeartBeatType_kRegist;  // kRgist
  heart_beat_info->addr = heart_beat_sender_addr;
  HeartBeatSender sender(detector_addr, heart_beat_info);
  auto send_status = sender.SendHeartBeat();
  ASSERT_EQ(send_status.IsOK(), true) << "heart beat fail" << std::endl;


  auto client_func = [=]() {
    const std::string input = "mkdir /a/b";
    Client client(input, addr);
    bool client_status = client.StartClient();
    ASSERT_EQ(true, client.StartClient()) << "start client error!" << std::endl;
    ;
  };
  std::thread t(client_func);
  t.join();
  detector_server.RunUntilAskedToQuit();
  server.RunUntilAskedToQuit();
}

}  // namespace master
}  // namespace ctgfs
