/*
 * author: wfgu(peter.wfgu@gmail.com)
 * */

#pragma once

#include <cstring>
#include <util/status.h>

namespace ctgfs {
namespace util {

// JSON type below are all JSON data structure that can be serialize 
// and deserialize.
enum JsonType {
  kJsonNull,
  kJsonFalse,
  kJsonTrue,
  kJsonNumber,
  kJsonString,
  kJsonArray,
  kJsonObect, 
};

struct JsonStream {
  const char* json_str;
};

struct JsonObject {
  double n;
  JsonType type; 

  double GetNumber() const {
    assert(type == JsonType::kJsonNumber);
    return n;
  }

  JsonType GetType() const {
    return type;
  }
};

// A simple implementation of object notation.
// 
class Json {
 public:
  static Status Parse(const char* json, JsonObject& result);
  static JsonType GetType(const Json& obj);

 private:
  static void parseWhitespace(JsonStream& jstream);
  static Status parseValue(JsonStream& jstream, JsonObject& result);
  static Status parseLiteral(JsonStream& jstream, JsonObject& result, const char* literal, JsonType expect_type);
  static Status parseNumber(JsonStream& jstream, JsonObject& result);
};


} // namespace ctgfs
} // namespace util
