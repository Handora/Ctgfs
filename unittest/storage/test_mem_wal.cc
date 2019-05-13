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
  uint64_t size;
  Log a(Log(1, storage::Log::PUT, "a", "b"));
  Log b(Log(2, storage::Log::PUT, "e", "g"));
  Log c(Log(3, storage::Log::PUT, "c", "j"));
  Log d(Log(4, storage::Log::PUT, "f", "k"));
  std::vector<Log> vec;
  util::Status s = wal.Init("./tmp/", [&vec](const Log &log) { vec.push_back(log); });
  EXPECT_TRUE(s.IsOK());
  s = wal.LogToDisk(a);
  EXPECT_TRUE(s.IsOK());
  s = wal.LogToDisk(c);
  EXPECT_TRUE(s.IsOK());
  size = wal.GetDiskLogSize();
  s = wal.LogToDisk(b);
  EXPECT_TRUE(s.IsOK());
  s = wal.LogToDisk(d);
  EXPECT_TRUE(s.IsOK());
  s = wal.LogCurrent(0);
  EXPECT_TRUE(s.IsOK());
  s = wal.Stop();
  EXPECT_TRUE(s.IsOK());
  s = wal.Init("./tmp/", [&vec](const Log& log) { vec.push_back(log); });
  EXPECT_EQ(4, vec.size());
  EXPECT_EQ(a.key, vec[0].key);
  EXPECT_EQ(a.value, vec[0].value);
  EXPECT_EQ(c.key, vec[1].key);
  EXPECT_EQ(c.value, vec[1].value);
  EXPECT_EQ(b.key, vec[2].key);
  EXPECT_EQ(b.value, vec[2].value);
  EXPECT_EQ(d.key, vec[3].key);
  EXPECT_EQ(d.value, vec[3].value);
  s = wal.LogCurrent(size);
  EXPECT_TRUE(s.IsOK());
  s = wal.Stop();
  EXPECT_TRUE(s.IsOK());
  vec.clear();
  s = wal.Init("./tmp/", [&vec](const Log& log) { vec.push_back(log); });
  EXPECT_EQ(2, vec.size());
  EXPECT_EQ(b.key, vec[0].key);
  EXPECT_EQ(b.value, vec[0].value);
  EXPECT_EQ(d.key, vec[1].key);
  EXPECT_EQ(d.value, vec[1].value);
}
}  // namespace kv
}  // namespace ctgfs
