/*
 * author: wfgu(peter.wfgu@gmail.com)
 * based on miloyip/json-tutorial(https://github.com/miloyip/json-tutorial)
 * */

#pragma once

#include <cstring>
#include <vector>
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

  char* stack;
  size_t size, top;
  void* Push(size_t len) {
    void* ret;
    assert(len > 0);
    if (top + len >= size) {
      if (size == 0) {
        size = 256;
      }
      while (top + len >= size) {
        size += size >> 1;
      }
      stack = (char*)realloc(stack, size);
    }
    ret = stack + top;
    top += len;
    return ret;
  }
  void* Pop(size_t len) {
    assert(top >= len); 
    return stack + (top -= len);
  }
};

struct JsonObject {
  JsonObject() : type(JsonType::kJsonNull) {}
  ~JsonObject() {
    if (type == JsonType::kJsonString) {
      if (s != nullptr) delete s;
      s = nullptr;
    }
  }

  void SetNull() {
    if (type == JsonType::kJsonString) {
      if (s != nullptr) delete s;
      s = nullptr;
    }
    type = JsonType::kJsonNull;
  }

  const char* GetString() const {
    assert(type == JsonType::kJsonString);
    return s;
  }
  size_t GetStrLen() const {
    assert(type == JsonType::kJsonString);
    return len;
  }
  void SetString(const char* str, size_t size) {
    assert(str != nullptr || len == 0);
    SetNull();
    s = new char[size + 1];
    memcpy(s, str, size);
    s[size] = '\0';
    len = size;
    type = JsonType::kJsonString;
  }


  void SetNumber(double num) {
    SetNull();
    n = num;
    type = JsonType::kJsonNumber;
  }

  double GetNumber() const {
    assert(type == JsonType::kJsonNumber);
    return n;
  }

  void SetBoolean(int b) {
    SetNull();
    type = b ? JsonType::kJsonTrue : JsonType::kJsonFalse;
  }

  bool GetBoolean() const {
    assert(type == JsonType::kJsonTrue || type == JsonType::kJsonFalse);
    return type == JsonType::kJsonTrue;
  }

  void SetType(JsonType t) {
    type = t;
  }

  JsonType GetType() const {
    return type;
  }

  union {
    struct {
      char *s;
      size_t len;
    };

    double n;
  };
  JsonType type; 
};

// A simple implementation of object notation.
// 
class Json {
 public:
  static Status Parse(const char* json, JsonObject& result);

 private:
  static void parseWhitespace(JsonStream& jstream);
  static Status parseValue(JsonStream& jstream, JsonObject& result);
  static Status parseLiteral(JsonStream& jstream, JsonObject& result, const char* literal, JsonType expect_type);
  static Status parseNumber(JsonStream& jstream, JsonObject& result);
  static Status parseString(JsonStream& jstream, JsonObject& result);
};


} // namespace ctgfs
} // namespace util
