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

Status Json::parseString(JsonStream& jstream, JsonObject& result) {
  assert(jstream.json_str[0] == '\"');
  jstream.json_str++;
  size_t head = jstream.top, len;
  const char* ptr = jstream.json_str;
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
          case '\"': *(char*)jstream.Push(sizeof(char)) = ('\"'); break;
          case '\\': *(char*)jstream.Push(sizeof(char)) = ('\\'); break;
          case '/': *(char*)jstream.Push(sizeof(char)) = ('/'); break;
          case '\b': *(char*)jstream.Push(sizeof(char)) = ('\b'); break;
          case '\f': *(char*)jstream.Push(sizeof(char)) = ('\f'); break;
          case '\n': *(char*)jstream.Push(sizeof(char)) = ('\n'); break;
          case '\r': *(char*)jstream.Push(sizeof(char)) = ('\r'); break;
          case '\t': *(char*)jstream.Push(sizeof(char)) = ('\t'); break;
          default:
            jstream.top = head;
            return Status::InvalidArgument("Parse Error. Invalid string.");
        } 
        break;
      case '\0':
        jstream.top = head;
        return Status::InvalidArgument("Parse Error. Invalid string.");
      default:
        if ((unsigned char)ch < 0x20) {
          jstream.top = head; 
          return Status::InvalidArgument("Parse Error. Invalid string.");
        }
        *(char*)jstream.Push(sizeof(char)) = ch;
    }
  }
}

Status Json::parseValue(JsonStream& jstream, JsonObject& result) {
  switch (*(jstream.json_str)) {
    case 't': return parseLiteral(jstream, result, "true", JsonType::kJsonTrue);
    case 'f': return parseLiteral(jstream, result, "false", JsonType::kJsonFalse);
    case 'n': return parseLiteral(jstream, result, "null", JsonType::kJsonNull);
    default: return parseNumber(jstream, result);
    case '\"': return parseString(jstream, result);
    case '\0': return Status::ExpectValue();
  }
}

} // namespace ctgfs
} // namespace util
