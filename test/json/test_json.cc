/*
* author: wfgu(peter.wfgu@gmail.com)
* */

#include <cstring>
#include <string>
#include <functional>
#include <gtest/gtest.h>
#include <util/status.h>
#include <util/json.h>

namespace ctgfs {
namespace util {

void TEST_LITERAL(json::type expect, const char* str) {
  jsontree::Json v; 
  v.SetBoolean(false);
  Status s = v.Parse(str);
  EXPECT_EQ(true, s.IsOK());
  EXPECT_EQ(expect, v.GetType());
}


void TEST_ERROR(const char* errmsg, const char* json) {
  jsontree::Json v; 
  v.SetBoolean(false);
  Status s = v.Parse(json);
  EXPECT_EQ(false, s.IsOK());
  EXPECT_EQ(errmsg, s.ErrorText()); 
  EXPECT_EQ((json::type::kNull), v.GetType());
}

TEST(JSON_literal_Test, Basic) {
  using namespace std;
 
  TEST_LITERAL(json::type::kNull, "null");
  TEST_LITERAL(json::type::kTrue, "true");
  TEST_LITERAL(json::type::kFalse, "false");
}

}  // namespace util 
}  // namespace ctgfs
