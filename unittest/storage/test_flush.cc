// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <gtest/gtest.h>
#include <storage/sstable/sst_meta.h>
#include <storage/sstable/sstable.h>
#include <storage/sstable/flusher.h>
#include <storage/log.h>
#include <util/util.h>

namespace ctgfs {
namespace sstable {

using namespace storage;

TEST(FlushTest, DiskIO) {
  util::MyRemoveDirectoryRecursively("./tmp");
  mkdir("./tmp", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  Log l = Log(10, storage::Log::PUT, "key", "value");
  SStable sst;
  sst.Init("tmp", "test", l);
  sst.Append(Log(1, storage::Log::PUT, "kea", "a"));
  sst.Append(Log(2, storage::Log::PUT, "keb", "b"));
  sst.Append(Log(3, storage::Log::PUT, "kec", "c"));
  sst.Append(l);
  sst.Flush();

  SSTIterator iter;
  sst.CreateIterator(iter);

  SSTFlusher *flusher = new SSTFlusher();
  flusher->Init();
  SStable tmp_sst;
  flusher->Flush("tmp", "test2", iter, l, tmp_sst);

  
  SSTIterator iter2;
  tmp_sst.CreateIterator(iter2);

  Log log;
  EXPECT_TRUE(iter2.HasNext());
  iter2.Next(log);
  EXPECT_EQ("kea", log.key);

  EXPECT_TRUE(iter2.HasNext());
  iter2.Next(log);
  EXPECT_EQ("keb", log.key);

  EXPECT_TRUE(iter2.HasNext());
  iter2.Next(log);
  EXPECT_EQ("kec", log.key);

  EXPECT_TRUE(iter2.HasNext());
  iter2.Next(log);
  EXPECT_EQ("key", log.key);

  EXPECT_FALSE(iter2.HasNext());

}

}  // namespace sstable
}  // namespace ctgfs
