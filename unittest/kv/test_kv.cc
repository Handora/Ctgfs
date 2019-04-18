// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <gtest/gtest.h>
#include <kv/kv.h>
#include <kv/raft_kv.h>
#include <memory>

namespace ctgfs {
namespace kv {

TEST(KVTest, Basic) {
  std::shared_ptr<KV> kv = std::make_shared<RaftKV>();

  std::string key = "key";
  std::string value = "value";
  std::string res;

  EXPECT_EQ(true, kv->Put(key, value));
  EXPECT_EQ(true, kv->Get(key, res));

  // TODO(Handora): After finish the kv
  // EXPECT_EQ(value, res);
}

}  // namespace kv
}  // namespace ctgfs
