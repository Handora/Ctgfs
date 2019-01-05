/*
 * author: wfgu(peter.wfgu@gmail.com)
 * based on miloyip/json-tutorial(https://github.com/miloyip/json-tutorial)
 * */

#include <cstring>
#include <cassert>
#include <cstdlib>
#include <cmath>
#include <string>
#include <util/json.h>
#include <util/status.h>

#define EXPECTCH(js, ch) do { assert(*js.json_str == (ch)); js.json_str++; } while(0)
#define PUTCH(js, ch) do { *(char*)js.Push(sizeof(char)) = (ch); } while(0)

namespace ctgfs {
namespace util {
  
Status Json::Parse(const char* json_str, JsonObject& result) {
  JsonStream jstream;
  jstream.json_str = json_str;
  jstream.stack = nullptr;
  jstream.size = jstream.top = 0;
  result.SetNull();
  parseWhitespace(jstream);
  Status parse_status = parseValue(jstream, result);  
  if (parse_status.IsOK()) {
    parseWhitespace(jstream);
    if (*(jstream.json_str) != '\0') {
      result.SetType(JsonType::kJsonNull);
      parse_status = Status::InvalidArgument("Not a single json in the stream.");
    }
  }
  assert(jstream.top == 0);
  free(jstream.stack);  

  return parse_status;
}

void Json::parseWhitespace(JsonStream& jstream) {
  const char* ptr = jstream.json_str;

  while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') {
    ++ptr;
  }

  jstream.json_str = ptr;
}

Status Json::parseLiteral(JsonStream& jstream, JsonObject& result, const char* literal, JsonType expect_type) {
  assert(jstream.json_str[0] == literal[0]);

  size_t i, slen = strlen(literal);  
  for (i = 0; i < slen; ++i) {
    if (jstream.json_str[i] != literal[i]) {
      return Status::InvalidArgument("Parse Failed. Invalid literal");
    }
  }
  jstream.json_str += i;
  result.SetType(expect_type);

  return Status::OK();
}

Status Json::parseNumber(JsonStream& jstream, JsonObject& result) {
  const char* ptr = jstream.json_str;

  if (*ptr == '-') ++ptr;

  if (*ptr == '0') {
    ++ptr; 
  } else {
    if (!(*ptr >= '1' && *ptr <= '9')) {
      return Status::InvalidArgument("Parse Error. Invalid number");
    }
    for (++ptr; (*ptr >= '0' && *ptr <= '9'); ++ptr);
  }

  if (*ptr == '.') {
    ++ptr;
    if (!(*ptr >= '0' && *ptr <= '9')) {
      return Status::InvalidArgument("Parse Error. Invalid number");
    }
    for (++ptr; (*ptr >= '0' && *ptr <= '9'); ++ptr);
  }

  if (*ptr == 'e' || *ptr == 'E') {
    ++ptr;
    if (*ptr == '+' || *ptr == '-') ++ptr;
    if (!(*ptr >= '0' && *ptr <= '9')) {
      return Status::InvalidArgument("Parse Error. Invalid number");
    }
    for (++ptr; (*ptr >= '0' && *ptr <= '9'); ++ptr);
  }

  errno = 0;
  double num = strtod(jstream.json_str, NULL);
  if (errno == ERANGE && (num == HUGE_VAL || num == -HUGE_VAL)) {
    return Status::InvalidArgument("Parse Error. Invalid number");
  }
  result.SetNumber(num);
  jstream.json_str = ptr;

  return Status::OK();
}

#define STRING_ERROR(msg) do { jstream.top = head; return Status::InvalidArgument(msg); } while(0)

Status Json::parseString(JsonStream& jstream, JsonObject& result) {
  EXPECTCH(jstream, '\"');

  const char* ptr;
  size_t head = jstream.top, len = 0;
  unsigned u, u2;
  ptr = jstream.json_str;

  while (true) {
    char ch = *ptr++; 
    switch (ch) {
      case '\"':
        len = jstream.top - head; 
        result.SetString((const char*)jstream.Pop(len), len);
        jstream.json_str = ptr;
        return Status::OK();
      case '\\':
        switch(*ptr++) {
          case '\"': PUTCH(jstream, '\"'); break;
          case '\\': PUTCH(jstream, '\\'); break;
          case '/': PUTCH(jstream, '/'); break;
          case 'b': PUTCH(jstream, '\b'); break;
          case 'f': PUTCH(jstream, '\f'); break;
          case 'n': PUTCH(jstream, '\n'); break;
          case 'r': PUTCH(jstream, '\r'); break;
          case 't': PUTCH(jstream, '\t'); break;
          case 'u':  // Unicode
            if (!(ptr = parseHex4(ptr, u))) {
              STRING_ERROR("Parse Error. Invalid Unicode.");
            }
            if (u >= 0xD800 && u <= 0xDBFF) {
              if (*ptr++ != '\\') 
                STRING_ERROR("Parse Error. Invalid Unicode.");
              if (*ptr++ != 'u') 
                STRING_ERROR("Parse Error. Invalid Unicode.");
              if (!(ptr = parseHex4(ptr, u2))) 
                STRING_ERROR("Parse Error. Invalid Unicode.");
              if (u2 < 0xDC00 || u2 > 0xDFFF)  
                STRING_ERROR("Parse Error. Invalid Unicode.");

              u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
            }
            encodeUTF8(jstream, u); 
            break;
          default:
            STRING_ERROR("Parse Error. Invalid string.");
        } 
        break;
      case '\0':
        STRING_ERROR("Parse Error. Invalid string.");
      default:
        if ((unsigned char)ch < 0x20) 
          STRING_ERROR("Parse Error. Invalid string.");
        PUTCH(jstream, ch);
    }
  }
}

const char* Json::parseHex4(const char* p, unsigned& u) {
  u = 0;
  for (int i = 0; i < 4; ++i) {
    char ch = *p++;
    u <<= 4;
    if (ch >= '0' && ch <= '9') u |= ch - '0';
    if (ch >= 'A' && ch <= 'F') u |= ch - ('A' - 10);
    if (ch >= 'a' && ch <= 'f') u |= ch - ('a' - 10);
    else return nullptr;
  }
  return p;
}

void Json::encodeUTF8(JsonStream& jstream, const unsigned& u) {
  if (u <= 0x7F) {
    PUTCH(jstream, u & 0xFF);
  } else if (u <= 0x7FF) {
    PUTCH(jstream, 0xC0 | ((u >> 6) & 0xFF));
    PUTCH(jstream, 0x80 | (u & 0x3F));
  } else if (u <= 0xFFFF) {
    PUTCH(jstream, 0xE0 | ((u >> 12) & 0xFF));
    PUTCH(jstream, 0x80 | ((u >> 6) & 0x3F));
    PUTCH(jstream, 0x80 | (u & 0x3F));
  } else {
    assert(u <= 0x10FFFF);
    PUTCH(jstream, 0xF0 | ((u >> 18) & 0xFF));
    PUTCH(jstream, 0x80 | ((u >> 12) & 0x3F));
    PUTCH(jstream, 0x80 | ((u >> 6) & 0x3F));
    PUTCH(jstream, 0x80 | (u & 0x3F));
  }
}

Status Json::parseArray(JsonStream& jstream, JsonObject& result) {
  EXPECTCH(jstream, '[');
  parseWhitespace(jstream); 
  size_t size = 0;

  if (*jstream.json_str == ']') { 
    jstream.json_str++; 
    result.SetType(JsonType::kJsonArray);
    result.size = 0;
    result.arr = nullptr;
    return Status::OK();
  }

  Status s;
  while (true) {
    JsonObject obj; 
    s = parseValue(jstream, obj);
    if (!s.IsOK()) 
      break;
    memcpy(jstream.Push(sizeof(JsonObject)), &obj, sizeof(JsonObject));
    size++;
    parseWhitespace(jstream); 
    if (*jstream.json_str == ',') {
      jstream.json_str++;
      parseWhitespace(jstream); 
    } else if (*jstream.json_str == ']') {
      jstream.json_str++; 
      result.SetType(JsonType::kJsonArray);
      result.size = size;
      size *= sizeof(JsonObject);
      memcpy(result.arr = (JsonObject*)malloc(size), jstream.Pop(size), size);
      return Status::OK();
    } else {
      s = Status::InvalidArgument("Parse error. Invalid array.");
      break;
    }
  }

  for (int i = 0; i < size; ++i) {
    ((JsonObject*)jstream.Pop(sizeof(JsonObject)))->SetNull();
  }

  return s;
}

Status Json::parseValue(JsonStream& jstream, JsonObject& result) {
  switch (*(jstream.json_str)) {
    case 't': return parseLiteral(jstream, result, "true", JsonType::kJsonTrue);
    case 'f': return parseLiteral(jstream, result, "false", JsonType::kJsonFalse);
    case 'n': return parseLiteral(jstream, result, "null", JsonType::kJsonNull);
    default: return parseNumber(jstream, result);
    case '"': return parseString(jstream, result);
    case '[': return parseArray(jstream, result);
    case '\0': return Status::ExpectValue();
  }
}

} // namespace ctgfs
} // namespace util
