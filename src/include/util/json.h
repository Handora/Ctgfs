/*
 * author: wfgu(peter.wfgu@gmail.com)
 * Based on miloyip/json-tutorial(https://github.com/miloyip/json-tutorial)
 * and Syopain/Json(https://github.com/Syopain/Json)
 * */

#pragma once

#include <string>
#include <memory>
#include <util/status.h>

namespace ctgfs {
namespace util {

namespace json {

enum type : int {
  kNull,
  kTrue,
  kFalse,
  kNumber,
  kString,
  kArray,
  kObject, 
};

class Value;

} // namespace json

namespace jsontree {

class Json final {
 public:
  Json() noexcept;
  ~Json() noexcept;
  Json(const Json& rhs) noexcept;
  Json& operator=(const Json& rhs) noexcept;
  Json(Json &&rhs) noexcept;
  Json& operator=(Json &&rhs) noexcept;
  void Swap(Json &rhs) noexcept;

  Status Parse(const std::string& content) noexcept;
  void Stringify(std::string& content) const noexcept;


  int GetType() const noexcept;
  void SetNull() noexcept;
  void SetBoolean(bool b) noexcept;

  double GetNumber() const noexcept;
  void SetNumber(double d) noexcept;

  const std::string GetString() const noexcept;
  void SetString(const std::string &str) noexcept;

  size_t GetArraySize() const noexcept;
  Json GetArrayElement(size_t index) const noexcept;
  void SetArray() noexcept;
  void PushBackArrayElement(const Json& val) noexcept;
  void PopBackArrayElement() noexcept;
  void InsertArrayElement(const Json& val, size_t index) noexcept;
  void RemoveArrayElement(size_t index, size_t count) noexcept;
  void ClearArray() noexcept;

  size_t GetObjectSize() const noexcept;
  const std::string& GetObjectKey(size_t index) const noexcept;
  size_t GetObjectKeyLength(size_t index) const noexcept;
  Json GetObjectValue(size_t index) const noexcept;
  void SetObjectValue(const std::string &key, const Json& val) noexcept;
  void SetObject() noexcept;
  long long FindObject(const std::string &key) const noexcept;
  void RemoveObject(size_t index) noexcept;
  void ClearObject() noexcept;
  
 private:
  std::unique_ptr<json::Value> v;

  friend bool operator==(const Json& lhs, const Json& rhs) noexcept;
  friend bool operator!=(const Json& lhs, const Json& rhs) noexcept;
}; 
bool operator==(const Json& lhs, const Json& rhs) noexcept;
bool operator!=(const Json& lhs, const Json& rhs) noexcept;
void swap(Json& lhs, Json& rhs) noexcept;

} // namespace jsontree


} // namespace ctgfs
} // namespace util
