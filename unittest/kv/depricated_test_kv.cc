// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <gtest/gtest.h>
#include <kv/kv.h>
#include <kv/raft_kv.h>
#include <memory>
#include <brpc/server.h>

namespace ctgfs {
namespace kv {

TEST(KVTest, Basic) {
  util::MyRemoveDirectoryRecursively("./tmp");
  std::string initial_conf = "";
  int port = 15122;
  for (int i = 0; i < 3; i++) {
    initial_conf += "127.0.0.1:" + std::to_string(port + i) + ":0,";
  }

  std::shared_ptr<KV> kv1 = std::make_shared<RaftKV>("kv", 15122, initial_conf);
  std::shared_ptr<KV> kv2 = std::make_shared<RaftKV>("kv", 15123, initial_conf);
  std::shared_ptr<KV> kv3 = std::make_shared<RaftKV>("kv", 15124, initial_conf);
  usleep(2000 * 1000);

  std::string key = "key";
  std::string value = "value";
  std::string res;

  bool ok = false;
  if (kv1->Put(key, value) == true) {
    ok = true;
  } else if (kv2->Put(key, value) == true) {
    ok = true;
  } else if (kv3->Put(key, value) == true) {
    ok = true;
  }

  EXPECT_EQ(true, ok);
  usleep(500 * 1000);

  EXPECT_EQ(true, kv1->Get(key, res));
  EXPECT_EQ(true, kv2->Get(key, res));
  EXPECT_EQ(true, kv3->Get(key, res));

  EXPECT_EQ(value, res);
}

TEST(KVTest, Query) {
  util::MyRemoveDirectoryRecursively("./tmp");
  std::string initial_conf = "";
  int port = 15122;
  for (int i = 0; i < 3; i++) {
    initial_conf += "127.0.0.1:" + std::to_string(port + i) + ":0,";
  }

  std::shared_ptr<KV> kv1 = std::make_shared<RaftKV>("kv", 15122, initial_conf);
  std::shared_ptr<KV> kv2 = std::make_shared<RaftKV>("kv", 15123, initial_conf);
  std::shared_ptr<KV> kv3 = std::make_shared<RaftKV>("kv", 15124, initial_conf);
  usleep(2000 * 1000);

  std::string key = "key";
  std::string value = "value";
  std::map<std::string, std::string> res;

  bool ok = false;
  if (kv1->Put(key, value) == true) {
    kv1->Put(key + "_1", value);
    kv1->Put(key + "_2", value);
    ok = true;
  } else if (kv2->Put(key, value) == true) {
    kv2->Put(key + "_1", value);
    kv2->Put(key + "_2", value);
    ok = true;
  } else if (kv3->Put(key, value) == true) {
    kv3->Put(key + "_1", value);
    kv3->Put(key + "_2", value);
    ok = true;
  }

  EXPECT_EQ(true, ok);
  usleep(500 * 1000);

  EXPECT_EQ(true, kv1->Query(key, res));
  EXPECT_EQ(true, kv2->Query(key, res));
  EXPECT_EQ(true, kv3->Query(key, res));

  EXPECT_EQ(2, res.size());
}

}  // namespace kv
}  // namespace ctgfs
