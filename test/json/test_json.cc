/*
* author: wfgu(peter.wfgu@gmail.com)
* */

#include <cstring>
#include <string>
#include <functional>
#include <gtest/gtest.h>
#include <util/json.h>

namespace ctgfs {
namespace util {

static std::string status;

void TEST_ERROR(const char* errmsg, const char* content) {
  jsontree::Json j; 
  j.SetBoolean(false);
  j.Parse(content, status);
  EXPECT_EQ(errmsg, status); 
  EXPECT_EQ((json::type::kNull), j.GetType());
}

void TEST_LITERAL(json::type expect, const char* content) {
  jsontree::Json j; 
  j.SetBoolean(false);
  j.Parse(content, status);
  EXPECT_EQ("parse ok", status); 
  EXPECT_EQ(expect, j.GetType());
}

TEST(JSON_literal_Test, Basic) {
  using namespace std;
 
  TEST_LITERAL(json::type::kNull, "null");
  TEST_LITERAL(json::type::kTrue, "true");
  TEST_LITERAL(json::type::kFalse, "false");
}

void TEST_NUMBER(double expect, const char* content) {
  jsontree::Json j;
  j.SetBoolean(false);
  j.Parse(content, status);
  EXPECT_EQ("parse ok", status); 
  EXPECT_EQ(json::type::kNumber, j.GetType());
  EXPECT_EQ(expect, j.GetNumber());
}

TEST(JSON_number_Test, Basic) {
  using namespace std;
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
  TEST_NUMBER(0.0, "1e-10000"); /* must underflow */
}

}  // namespace util 
}  // namespace ctgfs
