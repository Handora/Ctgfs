// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <gtest/gtest.h>
#include <util/util.h>
#include <sys/stat.h>
#define private public 
#include <storage/memtable/wal.h>

namespace ctgfs {
namespace memtable {

TEST(WalTest, Basic) {
  util::MyRemoveDirectoryRecursively("./tmp");
  mkdir("./tmp", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  Wal wal;
  Log a(Log(1, storage::Log::PUT, "a", "b"));
  Log b(Log(2, storage::Log::PUT, "e", "g"));
  Log c(Log(3, storage::Log::PUT, "c", "j"));
  Log d(Log(4, storage::Log::PUT, "f", "k"));
  util::Status s = wal.Init("./tmp/");
  EXPECT_TRUE(s.IsOK());
  s = wal.AppendToMLog(a);
  EXPECT_TRUE(s.IsOK());
  s = wal.AppendToMLog(c);
  EXPECT_TRUE(s.IsOK());
  s = wal.AppendToMLog(b);
  EXPECT_TRUE(s.IsOK());
  s = wal.AppendToMLog(d);
  EXPECT_TRUE(s.IsOK());
  s = wal.SyncToDLog();
  s = wal.Stop();
  EXPECT_TRUE(s.IsOK());
  s = wal.Init("./tmp/");
  EXPECT_EQ(4, wal.mlog_.size());
  EXPECT_EQ(a.key, wal.mlog_[0].key);
  EXPECT_EQ(a.value, wal.mlog_[0].value);
  EXPECT_EQ(c.key, wal.mlog_[1].key);
  EXPECT_EQ(c.value, wal.mlog_[1].value);
  EXPECT_EQ(b.key, wal.mlog_[2].key);
  EXPECT_EQ(b.value, wal.mlog_[2].value);
  EXPECT_EQ(d.key, wal.mlog_[3].key);
  EXPECT_EQ(d.value, wal.mlog_[3].value);
}
}  // namespace kv
}  // namespace ctgfs
