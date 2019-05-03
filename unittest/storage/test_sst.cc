// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <gtest/gtest.h>
#include <storage/sstable/sst_meta.h>
#include <storage/sstable/sstable.h>
#include <storage/log.h>
#include <util/util.h>

namespace ctgfs {
namespace sstable {

using namespace storage;

TEST(SSTMetaTest, DiskIO) {
  util::MyRemoveDirectoryRecursively("./tmp");
  mkdir("./tmp", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  Log l = Log(10, storage::Log::PUT, "key", "value");
  SSTMeta meta(l, l.Size() + 8, 128);
  SStable sst;
  sst.Init("tmp", "test", l);
  sst.Append(Log(1, storage::Log::PUT, "kea", "a"));
  sst.Append(Log(2, storage::Log::PUT, "keb", "b"));
  sst.Append(Log(3, storage::Log::PUT, "kec", "c"));
  sst.Append(l);
  sst.Flush();

  SSTIterator iter;
  sst.CreateIterator(iter);
  Log log;
  EXPECT_TRUE(iter.HasNext());
  iter.Next(log);
  EXPECT_EQ("kea", log.key);

  EXPECT_TRUE(iter.HasNext());
  iter.Next(log);
  EXPECT_EQ("keb", log.key);

  EXPECT_TRUE(iter.HasNext());
  iter.Next(log);
  EXPECT_EQ("kec", log.key);

  EXPECT_TRUE(iter.HasNext());
  iter.Next(log);
  EXPECT_EQ("key", log.key);

  EXPECT_FALSE(iter.HasNext());
}

}  // namespace sstable
}  // namespace ctgfs
