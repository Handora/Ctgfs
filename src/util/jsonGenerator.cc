/*
 * author: wfgu(peter.wfgu@gmail.com)                                                              
 * Based on miloyip/json-tutorial(https://github.com/miloyip/json-tutorial)
 * and Syopain/Json(https://github.com/Syopain/Json)
 * */

#include <assert.h>
#include <string>
#include "util/jsonGenerator.h"

namespace ctgfs {
namespace util {

namespace json {

Generator::Generator(const Value& val, std::string& result) 
  : res_(result) {
  res_.clear();
  stringifyValue(val);
}

void Generator::stringifyValue(const Value& v) {
  switch (v.GetType()) {
    case type::kNull: res_ += "null"; break;
    case type::kTrue: res_ += "true"; break;
    case type::kFalse: res_ += "false"; break;
    case type::kNumber: {
      
    }
    case type::kString: stringifyString(v.GetString()); break;
    case type::kArray: 
      res_ += '[';
      for (int i = 0; i < v.GetArraySize(); ++i) {
        if (i > 0) res_ += ',';
        stringifyValue(v.GetArrayElement(i));
      }
      res_ += ']';
      break;
    case type::kObject:
      res_ += '{';
      for (int i = 0; i < v.GetObjectSize(); ++i) {
        if (i > 0) res_ += ',';
        stringifyString(v.GetObjectKey(i));
        res_ += ':';
        stringifyValue(v.GetObjectValue(i));
      }
      res_ += '}';
      break;
  }
}

void Generator::stringifyString(const std::string& str) {
  res_ += '\"';
  for (auto ch : str) {
    switch (ch) {
      case '\"': res_ += "\\\""; break;
      case '\\': res_ += "\\\\"; break;
      case '\b': res_ += "\\b";  break;
      case '\f': res_ += "\\f";  break;
      case '\n': res_ += "\\n";  break;
      case '\r': res_ += "\\r";  break;
      case '\t': res_ += "\\t";  break;
      default:
      if (ch < 0x20) {
        char buffer[7] = {0};
        sprintf(buffer, "\\u%04X", ch);
        res_ += buffer;
      } else {
        res_ += ch;
      }
    }
  }
  res_ += '\"';
}

} // namespace json

} // namespace util
} // namespace ctgfs

