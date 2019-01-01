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

TEST(JSONliteralTest, Basic) {
  using namespace std;
  // test parse null
  JsonObject t0;
  t0.type = JsonType::kJsonTrue;
  Status s0 = Json::Parse("null", t0);
  EXPECT_EQ(true, s0.IsOK());
  EXPECT_EQ(JsonType::kJsonNull, t0.type); 

  // test parse true
  JsonObject t1;
  t1.type = JsonType::kJsonFalse;
  Status s1 = Json::Parse("true", t1);
  EXPECT_EQ(true, s1.IsOK());
  EXPECT_EQ(JsonType::kJsonTrue, t1.type); 

  // test parse false
  JsonObject t2;
  t2.type = JsonType::kJsonTrue;
  Status s2 = Json::Parse("false", t2);
  EXPECT_EQ(true, s2.IsOK());
  EXPECT_EQ(JsonType::kJsonFalse, t2.type); 
}

// test number
TEST(JSONNumberTest, Basic) {
  using namespace std;

  function<void(double, const char*)> TEST_NUMBER = [&](double f, const char* str) {
    JsonObject j;
    Status s = Json::Parse(str, j);
    EXPECT_EQ(true, s.IsOK());
    EXPECT_EQ(f, j.GetNumber());
  };

  TEST_NUMBER(0.0, "0");
  TEST_NUMBER(0.0, "-0");
  TEST_NUMBER(0.0, "-0.0");
  TEST_NUMBER(1.0, "1");
  TEST_NUMBER(-1.0, "-1");
  TEST_NUMBER(1.5, "1.5");
  TEST_NUMBER(-1.5, "-1.5");
  TEST_NUMBER(3.1416, "3.1416");
  TEST_NUMBER(1E10, "1E10");
  TEST_NUMBER(1e10, "1e10");
  TEST_NUMBER(1E+10, "1E+10");
  TEST_NUMBER(1E-10, "1E-10");
  TEST_NUMBER(-1E10, "-1E10");
  TEST_NUMBER(-1e10, "-1e10");
  TEST_NUMBER(-1E+10, "-1E+10");
  TEST_NUMBER(-1E-10, "-1E-10");
  TEST_NUMBER(1.234E+10, "1.234E+10");
  TEST_NUMBER(1.234E-10, "1.234E-10");
  TEST_NUMBER(0.0, "1e-10000"); 

  TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
  TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324" ); /* minimum denormal */
  TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
  TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308" );  /* Max subnormal double */
  TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
  TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308" );  /* Min normal positive double */
  TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
  TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308" );  /* Max double */
  TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");

}

TEST(JSONStringTest, Basic) {
  using namespace std;
  JsonObject j;
  j.SetString("Hello", 5);
  EXPECT_EQ(JsonType::kJsonString, j.GetType());
  EXPECT_EQ("Hello", string(j.GetString()));

  Status s = Json::Parse("\"Hello World!\"", j);
  EXPECT_EQ(true, s.IsOK());
  EXPECT_EQ("Hello World!", string(j.GetString()));
}

}  // namespace util 
}  // namespace ctgfs
