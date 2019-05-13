/*
 * author: wfgu(peter.wfgu@gmail.com)
 * */

#include <gtest/gtest.h>
#include <mvcc/mvcc.h>

namespace ctgfs {
namespace mvcc {


TEST(MVCCTest, Query) {
  std::shared_ptr<KV> kv = std::make_shared<MockKV>();
  std::shared_ptr<MVCC> mvcc = std::make_shared<SimpleMVCC>(kv);

  std::string key = "key";
  std::string value = "value";

  EXPECT_EQ(true, mvcc->Insert(key, value + "_9", 9));
  EXPECT_EQ(true, mvcc->Insert(key, value + "_6", 6));
  EXPECT_EQ(true, mvcc->Insert(key, value + "_3", 3));

  std::map<std::string, std::string> res;

  std::string ret;

  std::string au_3 = value + "_3";
  EXPECT_EQ(true, mvcc->Select(key, ret, 5));
  EXPECT_EQ(au_3, ret);

  ret.clear();
  std::string au_6 = value + "_6";
  EXPECT_EQ(true, mvcc->Select(key, ret, 8));
  EXPECT_EQ(au_6, ret);

  EXPECT_EQ(false, mvcc->Select(key, ret, 1));

}
TEST(MVCCTest, Delete) {
  std::shared_ptr<KV> kv = std::make_shared<MockKV>();
  std::shared_ptr<MVCC> mvcc = std::make_shared<SimpleMVCC>(kv);

  std::string key = "key";
  std::string value = "value";

  EXPECT_EQ(true, mvcc->Insert(key, value + "_1", 1));
  EXPECT_EQ(true, mvcc->Insert(key, value + "_2", 2));
  EXPECT_EQ(true, mvcc->Delete(key, 4));
  EXPECT_EQ(true, mvcc->Insert(key, value + "_5", 5));
  EXPECT_EQ(true, mvcc->Delete(key, 6));

  std::map<std::string, std::string> res;

  std::string ret;
  EXPECT_EQ(true, mvcc->Select(key, ret, 3));
  EXPECT_EQ(value + "_2", ret);

  EXPECT_EQ(false, mvcc->Select(key, ret, 4));
  EXPECT_EQ("", ret);

  EXPECT_EQ(true, mvcc->Select(key, ret, 5));
  EXPECT_EQ(value + "_5", ret);

  EXPECT_EQ(false, mvcc->Select(key, ret, 6));
  EXPECT_EQ("", ret);

  EXPECT_EQ(false, mvcc->Select(key, ret, 7));
  EXPECT_EQ("", ret);

}

}  // namespace mvcc
}  // namespace ctgfs
