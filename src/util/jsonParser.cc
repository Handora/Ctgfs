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
      throw(Exception("More than one json file"));
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
    case '\0': throw(Exception("parse expect value"));
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
      throw(Exception("parse invalid value"));
  }
  cur_ +=i ;
  val_.SetType(t);
}

void Parser::parseNumber() {
  const char* p = cur_;
  if (*p == '-') ++p;  

  if (*p == '0') ++p;
  else {
    if (!isdigit(*p)) throw (Exception("parse invalid value"));
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
  std::string s;
  parseStringRaw(s);
  val_.SetString(s);
}

void Parser::parseStringRaw(std::string& result) {
  expect(cur_, '\"');
  const char* p = cur_;
  unsigned u = 0, u2 = 0;
  while (*p != '\"') {
    if (*p == '\0') 
    throw(Exception("parse miss quotation mark"));
    if(*p == '\\' && ++p){
      switch(*p++) {
        case '\"': result += '\"'; break;
        case '\\': result += '\\'; break;
        case '/' : result += '/' ; break;
        case 'b' : result += '\b'; break;
        case 'f' : result += '\f'; break;
        case 'n' : result += '\n'; break;
        case 'r' : result += '\r'; break;
        case 't' : result += '\t'; break;
        case 'u' : 
        parseHex4(p, u);
        if (u >= 0xD800 && u <= 0xDBFF) {
          if (*p++ != '\\')
          throw(Exception("parse invalid unicode surrogate"));
          if (*p++ != 'u')
          throw(Exception("parse invalid unicode surrogate"));
          parseHex4(p, u2);
          if (u2 < 0xDC00 || u2 > 0xDFFF)
          throw(Exception("parse invalid unicode surrogate"));
          u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
        }
        parseEncodeUTF8(result, u);
        break;
        default: throw (Exception("parse invalid string escape"));
      }
    } else if ((unsigned char) *p < 0x20) {
      throw (Exception("parse invalid string char"));
    } else {
      result += *p++; 
    }
  }
  cur_ = ++p;
}

void Parser::parseHex4(const char*& p, unsigned &u) {
  u = 0;
  for (int i = 0; i < 4; ++i) {
    char ch = *p++;
    u <<= 4;
    if (isdigit(ch))
      u |= ch - '0';
    else if (ch >= 'A' && ch <= 'F')
      u |= ch - ('A' - 10);
    else if (ch >= 'a' && ch <= 'f')
      u |= ch - ('a' - 10);
    else throw(Exception("parse invalid unicode hex"));
  }
}

void Parser::parseEncodeUTF8(std::string& s, unsigned u) {
  if (u <= 0x7F) {
    s += static_cast<char> (u & 0xFF);
  } else if (u <= 0x7FF) {
    s += static_cast<char> (0xC0 | ((u >> 6) & 0xFF));
    s += static_cast<char> (0x80 | ( u   & 0x3F ));
  } else if (u <= 0xFFFF) {
    s += static_cast<char> (0xE0 | ((u >> 12) & 0xFF));
    s += static_cast<char> (0x80 | ((u >>  6) & 0x3F));
    s += static_cast<char> (0x80 | ( u        & 0x3F ));
  } else {
    assert(u <= 0x10FFFF);
    s += static_cast<char> (0xF0 | ((u >> 18) & 0xFF));
    s += static_cast<char> (0x80 | ((u >> 12) & 0x3F));
    s += static_cast<char> (0x80 | ((u >>  6) & 0x3F));
    s += static_cast<char> (0x80 | ( u        & 0x3F ));
  }
}

void Parser::parseArray() {
  expect(cur_, '[');
  parseWhitespace();
  std::vector<Value> ret;
  if (*cur_ == ']') {
    ++cur_;
    val_.SetArray(ret);
    return;
  }
  while (true) {
    try {
      parseValue();
    } catch(Exception) {
      val_.SetType(type::kNull);
      throw;
    }
    ret.push_back(val_);
    parseWhitespace();

    if (*cur_ == ',') {
      ++cur_;
      parseWhitespace();
    } else if (*cur_ == ']') {
      ++cur_;
      val_.SetArray(ret);
      return;
    } else {
      val_.SetType(type::kNull);
      throw(Exception("parse miss comma or square bracket"));
    }
  }
}

void Parser::parseObject() {
  expect(cur_, '{');
  parseWhitespace();
  std::vector<std::pair<std::string, Value>> ret;
  std::string key;
  if (*cur_ == '}') {
    ++cur_;
    val_.SetObject(ret);
    return;
  }
  while (true) {
    if (*cur_ != '\"')
      throw(Exception("parse miss key"));

    try {
      parseStringRaw(key);
    } catch(Exception) {
      throw(Exception("parse miss key"));
    }
    parseWhitespace();
    if (*cur_++ != ':') 
      throw(Exception("parse miss colon"));
    parseWhitespace();

    try {
      parseValue();
    } catch (Exception) {
      val_.SetType(type::kNull);
      throw;
    }
    ret.push_back(make_pair(key, val_));
    val_.SetType(type::kNull);
    key.clear();
    parseWhitespace();
    if (*cur_ == ',') {
      ++cur_;
      parseWhitespace();
    } else if (*cur_ == '}') {
      ++cur_;
      val_.SetObject(ret);
      return;
    } else {
      val_.SetType(type::kNull);
      throw(Exception("parse miss comma or curly bracket"));
    }
  }
}


} // namespace json

} // namespace util
} // namespace ctgfs

