/*
 * author: wfgu(peter.wfgu@gmail.com)                                                              
 * Based on miloyip/json-tutorial(https://github.com/miloyip/json-tutorial)
 * and Syopain/Json(https://github.com/Syopain/Json)
 * */

#include <cassert>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <util/jsonParse.h>
#include <util/jsonExcetion.h>

namespace ctgfs {
namespace util {

namespace json {
  
  inline void expect(const char*& c, char ch) {
    assert(*c == ch);
    ++c;
  }

  Parser::Parser(Value& val, const std::string& content) :
    val_(val), cur_(content.c_str()) {
    val_.SetType(type::kNull);
    parseWhitespace();
    parseValue();
    parseWhitespace();
      if (*cur_ != '\0') {
        val_.SetType(type::kNull);
        throw(Exception("More than one json file."));
      }
  }

  void Parser::parseValue() {
    switch (*cur_) {
      case 'n': parseLiteral("null", type::kNull); break;
      case 't': parseLiteral("true", type::kTrue); break;
      case 'f': parseLiteral("false", type::kFlase); break;
      case '\"': parseString(); break;
      case '[': parseArray(); break;
      case '{': parseObject(); break;
      default: parseNumber(); break;
      case '\0': throw(Exception("Parse expect value."));
    }
  }

  void Parser::parseWhitespace() noexcept {
    while (*cur_ == ' ' || *cur_ == '\t' || *cur_ == '\n' || *cur_ == '\r') {
          ++cur_;
    }
  }

  void parseValue() {

  }

  void parseLiteral(const char* literal, type t) {

  }

  void parseNumber() {

  }

  void parseString() {

  }

  void parseStringRaw(std::string& tmp) {

  }

  void parseHex4(const char*& p, unsigned &u) {

  }

  void parseEncodeUTF8(std::string& s, unsigned u) {

  }

  void parseArray() {

  }

  void parseObject() {

  }


} // namespace json

} // namespace util
} // namespace ctgfs

