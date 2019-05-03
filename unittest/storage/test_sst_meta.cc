// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <gtest/gtest.h>
#include <storage/sstable/sst_meta.h>
#include <storage/log.h>
#include <util/util.h>

namespace ctgfs {
namespace sstable {

using namespace storage;

TEST(SSTMetaTest, Basic) {
  Log l = Log(1, storage::Log::PUT, "key", "value");
  SSTMeta meta(l, l.Size() + 8, 128);
  std::string str;
  meta.Encode(str);
  SSTMeta other;
  other.Decode(str);
  EXPECT_EQ(1, other.last_log_.lsn);
  EXPECT_EQ("key", other.last_log_.key);
  EXPECT_EQ("value", other.last_log_.value);
  std::string other_str;
  other.Encode(other_str);
  EXPECT_EQ(str, other_str);
}

TEST(SSTMetaTest, DiskIO) {
  util::MyRemoveDirectoryRecursively("./tmp");
  mkdir("./tmp", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  Log l = Log(1, storage::Log::PUT, "key", "value");
  SSTMeta meta(l, l.Size() + 8, 128);
  int fd = open("tmp/test", O_CREAT | O_WRONLY, 0666);
  EXPECT_TRUE(fd > 0);
  uint64_t writed = 0;
  meta.Write(fd, writed);
  EXPECT_EQ(meta.Size() + sizeof(uint64_t), writed);
  SSTMeta other;
  close(fd);
  fd = open("tmp/test", O_RDONLY);
  EXPECT_TRUE(fd > 0);
  uint64_t readed = 0;
  other.Read(fd, readed);
  EXPECT_EQ(writed, readed);
  EXPECT_EQ(1, other.last_log_.lsn);
  EXPECT_EQ("key", other.last_log_.key);
  EXPECT_EQ("value", other.last_log_.value);
  close(fd);
}

}  // namespace sstable
}  // namespace ctgfs
