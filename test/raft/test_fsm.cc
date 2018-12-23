// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <gtest/gtest.h>
#include <raft/fsm.h>
#include <brpc/server.h>

namespace ctgfs {
namespace raft {

TEST(FSMTest, Basic) {
  brpc::Server server;

  util::Options options("basic", 15122);
  auto fsm = std::make_shared<RocksFSM>(options);

  braft::add_service(&server, 15122);
  server.Start(15122, NULL);

  util::Status s = fsm->Open();
  EXPECT_EQ(true, s.IsOK());

  auto waiter = std::make_shared<util::Waiter>();
  s = fsm->Put("key", "value", waiter);
  EXPECT_EQ(true, s.IsOK());

  s = waiter->Wait();
  EXPECT_EQ(true, s.IsOK());
}

}  // namespace parser
}  // namespace ctgfs
