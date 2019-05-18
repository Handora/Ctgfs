// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <gtest/gtest.h>
#include <util/util.h>
#include <sys/stat.h>
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
  std::vector<Log> &mlog = wal.GetMLog();
  EXPECT_EQ(4, mlog.size());
  EXPECT_EQ(a, mlog[0]);
  EXPECT_EQ(c, mlog[1]);
  EXPECT_EQ(b, mlog[2]);
  EXPECT_EQ(d, mlog[3]);
}
}  // namespace kv
}  // namespace ctgfs
