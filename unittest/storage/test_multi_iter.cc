// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <gtest/gtest.h>
#include <storage/sstable/sst_meta.h>
#include <storage/sstable/sstable.h>
#include <storage/multi_iter.h>
#include <storage/log.h>
#include <util/util.h>

namespace ctgfs {
namespace sstable {

using namespace storage;

TEST(SSTableTest, DiskIO) {
  util::MyRemoveDirectoryRecursively("./tmp");
  mkdir("./tmp", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  Log l = Log(10, storage::Log::PUT, "key", "value");
  SStable sst;
  sst.Init("tmp", "test", l);
  sst.Append(Log(7, storage::Log::PUT, "kea", "a"));
  sst.Append(Log(5, storage::Log::PUT, "keb", "b"));
  sst.Append(Log(6, storage::Log::PUT, "kec", "c"));
  sst.Append(l);
  sst.Flush();

  SSTIterator iter;
  sst.CreateIterator(iter);

  Log l2 = Log(4, storage::Log::PUT, "key", "value2");
  SStable sst2;
  sst2.Init("tmp", "test2", l2);
  sst2.Append(Log(1, storage::Log::PUT, "kea", "a2"));
  sst2.Append(Log(3, storage::Log::PUT, "kec", "c2"));
  sst2.Append(Log(2, storage::Log::PUT, "ked", "d"));
  sst2.Append(l2);
  sst2.Flush();

  SSTIterator iter2;
  sst2.CreateIterator(iter2);

  Log l3 = Log(14, storage::Log::PUT, "key", "value4");
  SStable sst3;
  sst3.Init("tmp", "test3", l3);
  sst3.Append(Log(13, storage::Log::PUT, "kec", "c3"));
  sst3.Append(Log(12, storage::Log::PUT, "ked", "d3"));
  sst3.Append(Log(11, storage::Log::PUT, "kee", "e"));
  sst3.Append(l3);
  sst3.Flush();

  SSTIterator iter3;
  sst3.CreateIterator(iter3);

  std::vector<Iterator*> iters;
  iters.push_back(&iter);
  iters.push_back(&iter2);
  iters.push_back(&iter3);
  MultiIter miter(iters);

  Log log;
  EXPECT_TRUE(miter.HasNext());
  miter.Next(log);
  EXPECT_EQ("kea", log.key);
  EXPECT_EQ("a", log.value);
  EXPECT_EQ(7, log.lsn);

  EXPECT_TRUE(miter.HasNext());
  miter.Next(log);
  EXPECT_EQ("keb", log.key);
  EXPECT_EQ("b", log.value);
  EXPECT_EQ(5, log.lsn);

  EXPECT_TRUE(miter.HasNext());
  miter.Next(log);
  EXPECT_EQ("kec", log.key);
  EXPECT_EQ("c3", log.value);
  EXPECT_EQ(13, log.lsn);

  EXPECT_TRUE(miter.HasNext());
  miter.Next(log);
  EXPECT_EQ("ked", log.key);
  EXPECT_EQ("d3", log.value);
  EXPECT_EQ(12, log.lsn);

  EXPECT_TRUE(miter.HasNext());
  miter.Next(log);
  EXPECT_EQ("kee", log.key);
  EXPECT_EQ("e", log.value);
  EXPECT_EQ(11, log.lsn);

  EXPECT_TRUE(miter.HasNext());
  miter.Next(log);
  EXPECT_EQ("key", log.key);
  EXPECT_EQ("value4", log.value);
  EXPECT_EQ(14, log.lsn);

  EXPECT_FALSE(miter.HasNext());
}

}  // namespace sstable
}  // namespace ctgfs
