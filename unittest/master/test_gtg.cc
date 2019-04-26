/*
* author: Chen Qian(qcdsr970209@gmail.com)
**/

#include <client/gtg_client.h>
#include <gtest/gtest.h>
#include <master/gtg.h>

namespace ctgfs {
namespace master {

TEST(MasterTest, Connect) {
  using namespace ctgfs::client;
  GTG server(6543);
  GTC client("6543");
  int ts;
  client.get_ts(ts);
  EXPECT_EQ(0, ts);
  client.get_ts(ts);
  EXPECT_EQ(1, ts);
  client.get_ts(ts);
  EXPECT_EQ(2, ts);
  client.get_ts(ts);
  EXPECT_EQ(3, ts);
}

}  // namespace master
}  // namespace ctgfs
