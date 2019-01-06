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
#include <util/jsonException.h>

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
      case 'n': parseLiteral("null", json::type::kNull); break;
      case 't': parseLiteral("true", json::type::kTrue); break;
      case 'f': parseLiteral("false", json::type::kFalse); break;
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

  void Parser::parseLiteral(const char* literal, type t) {
    expect(cur_, literal[0]);  
    size_t i;
    for (i = 0; literal[i + 1]; ++i) {
      if (cur_[i] != literal[i + 1]) 
        throw(Exception("Parse invalid value."));
    }
    cur_ +=i ;
    val_.SetType(t);
  }

  void Parser::parseNumber() {
    const char* p = cur_;
    if (*p == '-') ++p;  

    if (*p == '0') ++p;
    else {
      if (!isdigit(*p)) throw (Exception("parse invalid value."));
      while (isdigit(*++p));
    }

    if(*p == '.') {
      if (!isdigit(*++p)) throw (Exception("parse invalid value"));
        while (isdigit(*++p));
    }

    if(*p == 'e' || *p == 'E') {
      ++p;
      if (*p == '+' || *p == '-') ++p;
      if (!isdigit(*p)) throw (Exception("parse invalid value"));
      while (isdigit(*++p)) ;
    }

    errno = 0;
    double v = strtod(cur_, NULL);
    if (errno == ERANGE && (v == HUGE_VAL || v == -HUGE_VAL))
    throw (Exception("parse number too big"));
    val_.SetNumber(v);
    cur_ = p;
  }

  void Parser::parseString() {

  }

  void Parser::parseStringRaw(std::string& tmp) {

  }

  void Parser::parseHex4(const char*& p, unsigned &u) {

  }

  void Parser::parseEncodeUTF8(std::string& s, unsigned u) {

  }

  void Parser::parseArray() {

  }

  void Parser::parseObject() {

  }


} // namespace json

} // namespace util
} // namespace ctgfs

