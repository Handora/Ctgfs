// Authors: Chen Qian(qcdsr970209@gmail.com)

#include <string>
#include <memory>
#include <functional>
#include <set>

#include <gtest/gtest.h>
#include <util/guid.h>

namespace ctgfs {
namespace util {

TEST(GUIDTest, Basic) {
  using namespace std;
  set<size_t> S;
  for (int i = 0; i < 10000; ++i) {
    Guid id = NewGuid();
    size_t hash_value = hash<Guid>{}(id);
    S.insert(hash_value);
  }

  EXPECT_EQ(10000, (int)S.size()); 
}

}  // namespace util 
}  // namespace ctgfs
