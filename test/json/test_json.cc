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

void TEST_STRING(const char* expect, const char* content) {
  jsontree::Json j;
  j.Parse(content, status);
  EXPECT_EQ("parse ok", status); 
  EXPECT_EQ(json::type::kString, j.GetType());
  EXPECT_EQ(0, memcmp(expect, j.GetString().c_str(), j.GetString().size()));
}

TEST(JSON_string_Test, Basic) {
  TEST_STRING("", "\"\"");
  TEST_STRING("Hello", "\"Hello\"");

  TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
  TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");

  TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
  TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
  TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
  TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
  TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
  TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
}

TEST(JSON_array_Test, Basic) {
  using namespace json;

  jsontree::Json j;
  j.Parse("[  ]", status);
  EXPECT_EQ("parse ok", status);
  EXPECT_EQ(type::kArray, j.GetType());
  EXPECT_EQ(0, j.GetArraySize());

  j.Parse("[ null , false , true , 123 , \"abc\"  ]", status);
  EXPECT_EQ("parse ok", status);
  EXPECT_EQ(type::kArray, j.GetType());
  EXPECT_EQ(5, j.GetArraySize());
  EXPECT_EQ(type::kNull,   j.GetArrayElement(0).GetType());
  EXPECT_EQ(type::kFalse,  j.GetArrayElement(1).GetType());
  EXPECT_EQ(type::kTrue,   j.GetArrayElement(2).GetType());
  EXPECT_EQ(type::kNumber, j.GetArrayElement(3).GetType());
  EXPECT_EQ(type::kString, j.GetArrayElement(4).GetType());
  EXPECT_EQ(123.0, j.GetArrayElement(3).GetNumber());
  EXPECT_EQ("abc", j.GetArrayElement(4).GetString());

  j.Parse("[ [  ] , [ 0  ] , [ 0 , 1  ] , [ 0 , 1 , 2  ]  ]", status);
  EXPECT_EQ("parse ok", status);
  EXPECT_EQ(type::kArray, j.GetType());
  EXPECT_EQ(4, j.GetArraySize());
  for(int i = 0; i < 4; ++i) {
    jsontree::Json a = j.GetArrayElement(i);
    EXPECT_EQ(type::kArray, a.GetType());
    EXPECT_EQ(i, a.GetArraySize());
    for(int j = 0; j < i; ++j) {
      jsontree::Json e = a.GetArrayElement(j);
      EXPECT_EQ(type::kNumber, e.GetType());
      EXPECT_EQ((double)j, e.GetNumber());
    }
  }
}

}  // namespace util 
}  // namespace ctgfs
