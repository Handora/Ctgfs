// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <gtest/gtest.h>
#include <kv/kv.h>
#include <kv/mock_kv.h>
#include <memory>

namespace ctgfs {
namespace kv {

TEST(KVTest, Basic) {
  std::shared_ptr<KV> kv = std::make_shared<MockKV>();

  std::string key = "key";
  std::string value = "value";
  std::string res;

  EXPECT_EQ(true, kv->Put(key, value));
  EXPECT_EQ(true, kv->Get(key, res));

  EXPECT_EQ(value, res);
}

TEST(KVTest, Query) {
  std::shared_ptr<KV> kv = std::make_shared<MockKV>();

  std::string key = "key";
  std::string value = "value";

  EXPECT_EQ(true, kv->Put(key + "_5", value + "5"));
  EXPECT_EQ(true, kv->Put(key + "_1", value + "1"));
  EXPECT_EQ(true, kv->Put(key + "_2", value + "2"));
  std::map<std::string, std::string> res;
  EXPECT_EQ(true, kv->Query(key, res));

  EXPECT_EQ(3, res.size());
  EXPECT_EQ(value + "1", res[key + "_1"]);
  EXPECT_EQ(value + "2", res[key + "_2"]);
  EXPECT_EQ(value + "5", res[key + "_5"]);
}
}  // namespace kv
}  // namespace ctgfs
