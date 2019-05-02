// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <gtest/gtest.h>
#include <util/util.h>
#include <sys/stat.h>
#define private public 
#include <wal/wal.h>

namespace ctgfs {
namespace wal {

TEST(WalTest, Basic) {
  util::MyRemoveDirectoryRecursively("./tmp");
  mkdir("./tmp", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  Wal wal;
  Log a(Log(WalType::PUT, "a", "b"));
  Log b(Log(WalType::PUT, "e", "g"));
  Log c(Log(WalType::PUT, "c", "j"));
  Log d(Log(WalType::PUT, "f", "k"));
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
  EXPECT_EQ(a, wal.mlog_[0]);
  EXPECT_EQ(c, wal.mlog_[1]);
  EXPECT_EQ(b, wal.mlog_[2]);
  EXPECT_EQ(d, wal.mlog_[3]);
}
}  // namespace kv
}  // namespace ctgfs
