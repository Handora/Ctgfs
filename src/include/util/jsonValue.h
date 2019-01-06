/*
 * author: wfgu(peter.wfgu@gmail.com)
 * Based on miloyip/json-tutorial(https://github.com/miloyip/json-tutorial)
 * and Syopain/Json(https://github.com/Syopain/Json)
 * */

#pragma once

#include <string>
#include <vector>
#include <utility>
#include <util/json.h>

namespace ctgfs {
namespace util {

namespace json {

class Value final {
 public:
  Value() noexcept;
  Value(const Value& rhs) noexcept;
  Value& operator=(const Value& rhs) noexcept;
  ~Value() noexcept;

  void Parse(const std::string& content);
  void Stringify(std::string& content) const noexcept;

  int GetType() const noexcept;
  void SetType(type t) noexcept;

  double GetNumber() const noexcept;
  void SetNumber(double d) noexcept;

  const std::string& GetString() const noexcept;
  void SetString(const std::string &str) noexcept;

  size_t GetArraySize() const noexcept;
  const Value& GetArrayElement(size_t index) const noexcept;
  void SetArray(const std::vector<Value> &arr) noexcept;
  void PushBackArrayElement(const Value& val) noexcept;
  void PopBackArrayElement() noexcept;
  void InsertArrayElement(const Value &val, size_t index) noexcept;
  void RemoveArrayElement(size_t index, size_t count) noexcept;
  void ClearArray() noexcept;

  size_t GetObjectSize() const noexcept;
  const std::string& GetObjectKey(size_t index) const noexcept;
  size_t GetObjectKeyLength(size_t index) const noexcept;
  const Value& GetObjectValue(size_t index) const noexcept;
  void SetObjectValue(const std::string &key, const Value &val) noexcept;
  void SetObject(const std::vector<std::pair<std::string, Value>> &obj) noexcept;
  long long FindObject(const std::string &key) const noexcept;
  void RemoveObject(size_t index) noexcept;
  void ClearObject() noexcept;

 private:
  void init(const Value& rhs) noexcept;
  void free() noexcept;

  json::type type_ = json::kNull;
  union {
    double num_;
    std::string str_;
    std::vector<Value> arr_;
    std::vector<std::pair<std::string, Value>> obj_;
  };

  friend bool operator==(const Value& lhs, const Value& rhs) noexcept;
}; 
bool operator==(const Value& lhs, const Value& rhs) noexcept;
bool operator!=(const Value& lhs, const Value& rhs) noexcept;

} // namespace json

} // namespace ctgfs
} // namespace util
