// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <brpc/server.h>
#include <gtest/gtest.h>
#include <raft/fsm.h>

namespace ctgfs {
namespace raft {

TEST(FSMTest, Basic) {
  brpc::Server server;

  util::Options options("basic", 15122);
  options.initial_conf = "127.0.0.1:15122:0,";
  auto fsm = std::make_shared<RocksFSM>(options);

  braft::add_service(&server, 15122);
  server.Start(15122, NULL);

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
}

}  // namespace raft
}  // namespace ctgfs
