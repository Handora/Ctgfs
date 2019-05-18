// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <gtest/gtest.h>
#include <storage/memtable/memtable.h>
#include <storage/sstable/sstable.h>
#include <storage/multi_iter.h>
#include <storage/log.h>
#include <util/util.h>

namespace ctgfs {
namespace memtable {

using namespace storage;

TEST(MemTableTest, Memtable) {
  util::MyRemoveDirectoryRecursively("./tmp");
  mkdir("./tmp", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  Memtable mt(10, "./tmp");
  mt.Init();

  Log l = Log(10, storage::Log::PUT, "key", "value");
  mt.Add(Log(3, storage::Log::PUT, "kec", "c"));
  mt.Add(Log(1, storage::Log::PUT, "kea", "a"));
  mt.Add(l);
  mt.Add(Log(2, storage::Log::PUT, "keb", "b"));
  SSTMgr &mgr = mt.GetSSTMgr();

  Log log;
  for (uint64_t i = 0; i < mgr.GetSSTs().size(); i++) {
    SStable &sst = mgr.GetSSTs()[i];

    SSTIterator iter;
    sst.CreateIterator(iter);

    EXPECT_TRUE(iter.HasNext());
    iter.Next(log);
    EXPECT_FALSE(iter.HasNext());
  }
  std::vector<Iterator*> piters;
  for (uint64_t i = 0; i < mgr.GetSSTs().size(); i++) {
    SStable &sst = mgr.GetSSTs()[i];

    SSTIterator *iter = new SSTIterator();
    sst.CreateIterator(*iter);

    piters.push_back(iter);
  }
  MultiIter miters(piters);
  
  EXPECT_TRUE(miters.HasNext());
  miters.Next(log);
  EXPECT_EQ("kea", log.key);

  EXPECT_TRUE(miters.HasNext());
  miters.Next(log);
  EXPECT_EQ("keb", log.key);

  EXPECT_TRUE(miters.HasNext());
  miters.Next(log);
  EXPECT_EQ("kec", log.key);

  EXPECT_TRUE(miters.HasNext());
  miters.Next(log);
  EXPECT_EQ("key", log.key);

}

}  // namespace sstable
}  // namespace ctgfs
