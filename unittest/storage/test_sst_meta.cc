// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <gtest/gtest.h>
#include <storage/sstable/sst_meta.h>
#include <storage/log.h>

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

}  // namespace sstable
}  // namespace ctgfs
