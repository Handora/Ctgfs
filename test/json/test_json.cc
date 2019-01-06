/*
* author: wfgu(peter.wfgu@gmail.com)
* */

#include <cstring>
#include <string>
#include <functional>
#include <string>
#include <iostream>
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

TEST(JSON_object_Test, Basic) {
  std::cerr << "Start object parse test" << std::endl;

  jsontree::Json v;

  v.Parse(" {  } ", status);
  EXPECT_EQ("parse ok", status);
  EXPECT_EQ(json::type::kObject, v.GetType());
  EXPECT_EQ(0, v.GetObjectSize());

  v.Parse(" { "
          "\"n\" : null , "
          "\"f\" : false , "
          "\"t\" : true , "
          "\"i\" : 123 , "
          "\"s\" : \"abc\", "
          "\"a\" : [ 1, 2, 3  ],"
          "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3  }"
          " } ", status);

  EXPECT_EQ("parse ok", status);
  EXPECT_EQ(7, v.GetObjectSize());

  EXPECT_EQ("n", v.GetObjectKey(0));
  EXPECT_EQ(json::type::kNull, v.GetObjectValue(0).GetType());

  EXPECT_EQ("f", v.GetObjectKey(1));
  EXPECT_EQ(json::type::kFalse, v.GetObjectValue(1).GetType());

  EXPECT_EQ("t", v.GetObjectKey(2));
  EXPECT_EQ(json::type::kTrue, v.GetObjectValue(2).GetType());

  EXPECT_EQ("i", v.GetObjectKey(3));
  EXPECT_EQ(json::type::kNumber, v.GetObjectValue(3).GetType());
  EXPECT_EQ(123.0, v.GetObjectValue(3).GetNumber());

  EXPECT_EQ("s", v.GetObjectKey(4));
  EXPECT_EQ(json::type::kString, v.GetObjectValue(4).GetType());
  EXPECT_EQ("abc", v.GetObjectValue(4).GetString());

  EXPECT_EQ("a", v.GetObjectKey(5));
  EXPECT_EQ(json::type::kArray, v.GetObjectValue(5).GetType());
  EXPECT_EQ(3, v.GetObjectValue(5).GetArraySize());
  for (int i = 0; i < 3; ++i) {
    jsontree::Json e = v.GetObjectValue(5).GetArrayElement(i);
    EXPECT_EQ(json::type::kNumber, e.GetType());
    EXPECT_EQ(i + 1.0, e.GetNumber());
  }

  EXPECT_EQ("o", v.GetObjectKey(6));
  {
    jsontree::Json o = v.GetObjectValue(6);
    EXPECT_EQ(json::type::kObject, o.GetType());
    for(int i = 0; i < 3; ++i) {
      jsontree::Json ov = o.GetObjectValue(i);
      EXPECT_EQ('1' + i, (o.GetObjectKey(i))[0]);
      EXPECT_EQ(1, o.GetObjectKeyLength(i));
      EXPECT_EQ(json::type::kNumber, ov.GetType());
      EXPECT_EQ(i + 1.0, ov.GetNumber());
    }
  }

  std::cerr << "End object parse test" << std::endl;
}  

} // namespace util 
} // namespace ctgfs
