// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <gtest/gtest.h>
#include <brpc/server.h>
#include <raft/fsm.h>
#include <util/util.h>

namespace ctgfs {
namespace raft {

/*
TEST(FSMTest, Basic) {
  util::MyRemoveDirectoryRecursively("./tmp");
  brpc::Server server;

  int port = 15122;
  util::Options options("basic", port);
  options.initial_conf = "127.0.0.1:" + std::to_string(port) + ":0,";
  std::cout << options.initial_conf << std::endl;
  auto fsm = std::make_shared<RocksFSM>(options);

  int res = braft::add_service(&server, port);
  EXPECT_EQ(0, res);

  res = server.Start(port, NULL);
  EXPECT_EQ(0, res);

  util::Status s = fsm->Open();
  EXPECT_EQ(true, s.IsOK());

  usleep(1000 * 1000);
  auto waiter = std::make_shared<util::Waiter>();
  s = fsm->Put("key", "value", waiter);
  EXPECT_EQ(true, s.IsOK());

  s = waiter->Wait();
  EXPECT_EQ(true, s.IsOK());

  std::string value;
  s = fsm->Get("key", value, waiter);
  EXPECT_EQ(true, s.IsOK());
  EXPECT_EQ("value", value);

  fsm->Close();
}
*/

TEST(FSMTest, Multiple) {
  util::MyRemoveDirectoryRecursively("./tmp");

  int port = 15122;
  std::string initial_conf = "";
  for (int i = 0; i < 3; i++) {
    initial_conf += "127.0.0.1:" + std::to_string(port + i) + ":0,";
  }

  std::vector<std::shared_ptr<RocksFSM>> fsms;
  std::vector<brpc::Server*> servers;

  for (int i = 0; i < 3; i++) {
    auto server = new brpc::Server();
    util::Options options("basic", port + i);
    options.initial_conf = initial_conf;
    options.log_uri = "local://tmp/fsm" + std::to_string(i) + "/log";
    options.raft_meta_uri =
        "local://tmp/fsm" + std::to_string(i) + "/raft_meta";
    options.snapshot_uri = "local://tmp/fsm" + std::to_string(i) + "/snapshot";
    options.rocksdb_path = "tmp/rocksdb" + std::to_string(i);

    auto fsm = std::make_shared<RocksFSM>(options);
    fsms.push_back(fsm);

    int res = braft::add_service(server, port + i);
    EXPECT_EQ(0, res);

    util::Status s = fsm->Open();
    std::cout << s.ToString() << std::endl;
    EXPECT_EQ(true, s.IsOK());
    res = server->Start(port + i, NULL);
    EXPECT_EQ(0, res);
    servers.push_back(server);
  }

  usleep(2000 * 1000);
  auto waiter = std::make_shared<util::Waiter>();

  bool be_leader = false;
  for (int i = 0; i < 3; i++) {
    if (fsms[i]->IsLeader()) {
      be_leader = true;
      auto fsm = fsms[i];

      util::Status s = fsm->Put("key_1", "value", waiter);
      EXPECT_EQ(true, s.IsOK());

      s = waiter->Wait();
      EXPECT_EQ(true, s.IsOK());

      waiter = std::make_shared<util::Waiter>();
      s = fsm->Put("key_2", "value2", waiter);
      EXPECT_EQ(true, s.IsOK());

      s = waiter->Wait();
      EXPECT_EQ(true, s.IsOK());

      waiter = std::make_shared<util::Waiter>();
      s = fsm->Put("key_3", "value3", waiter);
      EXPECT_EQ(true, s.IsOK());

      s = waiter->Wait();
      EXPECT_EQ(true, s.IsOK());

      std::string value;
      s = fsm->Get("key_1", waiter);
      EXPECT_EQ(true, s.IsOK());
      s = fsm->LocalGet("key_1", value);
      EXPECT_EQ(true, s.IsOK());
      EXPECT_EQ("value", value);

      std::map<std::string, std::string> values;
      s = fsm->LocalQuery("key", values);
      for (auto iter = values.begin(); iter != values.end(); iter++) {
        std::cout << iter->first << " " << iter->second << std::endl;
      }
      EXPECT_EQ(true, s.IsOK());
      EXPECT_EQ(3, values.size());
      EXPECT_EQ("value", values["key_1"]);
      EXPECT_EQ("value2", values["key_2"]);
      EXPECT_EQ("value3", values["key_3"]);
    }
  }
  EXPECT_EQ(true, be_leader);

  for (auto fsm : fsms) {
    fsm->Close();
  }
}

}  // namespace raft
}  // namespace ctgfs
