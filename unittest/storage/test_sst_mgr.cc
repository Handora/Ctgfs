// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <gtest/gtest.h>
#include <storage/sstable/sst_meta.h>
#include <storage/sstable/sstable.h>
#include <storage/log.h>
#include <util/util.h>
#include <storage/sstable/sst_mgr.h>

namespace ctgfs {
namespace sstable {

using namespace storage;

TEST(SSTMgr, Basic) {
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

  SSTMgr mgr("tmp");
  mgr.Init();
  for (int i = 0; i < 3; i++) {
    SSTIterator iter;
    sst.CreateIterator(iter);

    mgr.Flush(iter, l);
  }
  EXPECT_EQ(3, mgr.GetSSTs().size());

  Log log;
  
  for (int i = 0; i < 3; i++) { 
    SStable sst = mgr.GetSSTs()[i];
    SSTIterator iter;
    sst.CreateIterator(iter);

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
  SSTMgr mgr2("tmp");
  mgr2.Init();

  EXPECT_EQ(3, mgr2.GetSSTs().size());

  for (int i = 0; i < 3; i++) { 
    SStable sst = mgr2.GetSSTs()[i];
    SSTIterator iter;
    sst.CreateIterator(iter);

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
}

}  // namespace sstable
}  // namespace ctgfs
