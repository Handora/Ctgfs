/*
 * author: wfgu(peter.wfgu@gmail.com)
 * based on miloyip/json-tutorial(https://github.com/miloyip/json-tutorial)
 * */

#include <util/json.h>
#include <util/jsonValue.h>
#include <util/jsonException.h>

namespace ctgfs {
namespace util {

namespace jsontree {
  Json::Json() noexcept : v(new json::Value) {  }
  Json::~Json() noexcept {  }
  Json::Json(const Json& rhs) noexcept {
    v.reset(new json::Value(*(rhs.v)));
  }
  Json& Json::operator=(const Json& rhs) noexcept {
    v.reset(new json::Value(*(rhs.v)));
  }
  Json::Json(Json&& rhs) noexcept {
    v.reset(rhs.v.release());
  }
  Json& Json::operator=(Json&& rhs) noexcept {
    v.reset(rhs.v.release());
  }

  void Json::Swap(Json& rhs) noexcept {
    std::swap(v, rhs.v);
  }

  void swap(Json& lhs, Json& rhs) noexcept {
    lhs.Swap(rhs);
  }

  void Json::Parse(const std::string& content, std::string& status) noexcept {
    try {
      Parse(content);
      status = "parse ok";
    } catch (const json::Exception& msg) {
      status = msg.what();
    } catch (...) {

    }
  }

  void Json::Parse(const std::string& content) const noexcept {
    v->Parse(content);
  }

  void Json::Stringify(std::string& content) const noexcept {
    v->Stringify(content); 
  }

  int Json::GetType() const noexcept {
    if (v == nullptr) 
      return json::type::kNull;
    return v->GetType();
  }

  void Json::SetNull() noexcept {
    v->SetType(json::type::kNull);
  }

  void Json::SetBoolean(bool b) noexcept {
    v->SetType(b ? json::type::kTrue : json::type::kFalse);
  }

  double Json::GetNumber() const noexcept {
    return v->GetNumber();
  }

  void Json::SetNumber(double d) noexcept {
    v->SetNumber(d);
  }

  const std::string Json::GetString() const noexcept {
    return v->GetString();
  }

  void Json::SetString(const std::string& str) noexcept {
    v->SetString(str);
  }

  size_t Json::GetArraySize() const noexcept {
    return v->GetArraySize();
  }

  Json Json::GetArrayElement(size_t index) const noexcept {
    Json ret;
    ret.v.reset(new json::Value(v->GetArrayElement(index)));
    return ret;
  }

  void Json::SetArray() noexcept {
    v->SetArray(std::vector<json::Value>{});
  }

  void Json::PushBackArrayElement(const Json& val) noexcept {
    v->PushBackArrayElement(*val.v);
  }  

  void Json::PopBackArrayElement() noexcept {
    v->PopBackArrayElement();
  }

  void Json::InsertArrayElement(const Json& val, size_t index) noexcept {
    v->InsertArrayElement(*val.v, index);
  }

  void Json::RemoveArrayElement(size_t index, size_t cnt) noexcept {
    v->RemoveArrayElement(index, cnt);
  }

  void Json::ClearArray() noexcept {
    v->ClearArray();
  }

  void Json::SetObject() noexcept {
    v->SetObject(std::vector<std::pair<std::string, json::Value>>{});
  }

  size_t Json::GetObjectSize() const noexcept {
    return v->GetObjectSize();
  }

  const std::string& Json::GetObjectKey(size_t index) const noexcept {
    return v->GetObjectKey(index);
  }

  size_t Json::GetObjectKeyLength(size_t index) const noexcept {
    return v->GetObjectKeyLength(index);
  }

  Json Json::GetObjectValue(size_t index) const noexcept {
    Json ret;
    ret.v.reset(new json::Value(v->GetObjectValue(index)));
    return ret;
  }

  void Json::SetObjectValue(const std::string& key, const Json& val) noexcept {
    v->SetObjectValue(key, *val.v);
  }

  long long Json::FindObject(const std::string& key) const noexcept {
    return v->FindObject(key);
  }

  void Json::RemoveObject(size_t index) noexcept {
    v->RemoveObject(index);
  }

  void Json::ClearObject() noexcept {
    v->ClearObject();
  }

  bool operator==(const Json& lhs, const Json& rhs) noexcept {
    return *lhs.v == *rhs.v;
  }

  bool operator!=(const Json& lhs, const Json& rhs) noexcept {
    return *lhs.v != *rhs.v;
  }

} // namespace jsontree

} // namespace ctgfs
} // namespace util
