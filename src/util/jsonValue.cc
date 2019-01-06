/*
 * author: wfgu(peter.wfgu@gmail.com)                                                              
 * Based on miloyip/json-tutorial(https://github.com/miloyip/json-tutorial)
 * and Syopain/Json(https://github.com/Syopain/Json)
 * */

#include <assert.h>
#include <string>
#include "util/jsonValue.h"
#include "util/jsonParse.h"
#include "util/jsonGenerator.h"

namespace ctgfs {
namespace util {

namespace json {
  Value::Value() noexcept : num_(0) {  }

  Value::Value(const Value& rhs) noexcept {
    init(rhs);
  }

  Value& Value::operator=(const Value& rhs) noexcept {
    free();
    init(rhs);
  }

  Value::~Value() noexcept {
    free();
  }

  void Value::init(const Value& rhs) noexcept {
    type_ = rhs.type_;
    num_ = 0;
    switch (type_) {
      case json::kNumber: 
        num_ = rhs.num_;
        break;
      case json::kString:
        new(&str_) std::string(rhs.str_);
        break;
      case json::kArray:
        new(&arr_) std::vector<Value>(rhs.arr_);
        break;
      case json::kObject:
        new(&obj_) std::vector<std::pair<std::string, Value>>(rhs.obj_);
        break;
    }
  }

  void Value::free() noexcept {
    using namespace std;
    switch (type_) {
      case json::kString:
        str_.~string();
        break;
      case json::kArray:
        arr_.~vector<Value>();
        break;
      case json::kObject:
        obj_.~vector<pair<string, Value>>();
        break;
    }
  }

  int Value::GetType() const noexcept {
    return type_;
  }
  void Value::SetType(type t) noexcept {
    free();
    type_ = t;
  }

  double Value::GetNumber() const noexcept {
    assert(type_ == type::kNumber);
    return num_;
  }
  void Value::SetNumber(double d) noexcept {
    free();
    type_ = type::kNumber;
    num_ = d;
  }

  const std::string& Value::GetString() const noexcept {
    assert(type_ == type::kString);
    return str_;
  }
  void Value::SetString(const std::string& str) noexcept {
    if (type_ == type::kString) {
      str_ = str;
    } else {
      free();
      type_ = type::kString;
      new(&str_) std::string(str);
    }
  }

  size_t Value::GetArraySize() const noexcept {
    assert(type_ == type::kArray);
    return arr_.size();
  }
  const Value& Value::GetArrayElement(size_t index) const noexcept {
    assert(type_ == type::kArray);
    return arr_[index];
  }
  void Value::SetArray(const std::vector<Value>& arr) noexcept {
    if (type_ == type::kArray) {
      arr_ = arr;
    } else {
      free();
      type_ = json::kArray;
      new(&arr_) std::vector<Value>(arr);
    }
  }
  void Value::PushBackArrayElement(const Value& val) noexcept {
    assert(type_ == type::kArray);
    arr_.push_back(val);
  }
  void Value::PopBackArrayElement() noexcept {
    assert(type_ == type::kArray);
    arr_.pop_back();
  }
  void Value::InsertArrayElement(const Value& val, size_t index) noexcept {
    assert(type_ == type::kArray);
    arr_.insert(arr_.begin() + index, val);
  }
  void Value::RemoveArrayElement(size_t index, size_t cnt) noexcept {
    assert(type_ == type::kArray);
    arr_.erase(arr_.begin() + index, arr_.begin() + index + cnt);
  }
  void Value::ClearArray() noexcept {
    assert(type_ == type::kArray);
    arr_.clear();
  }

  size_t Value::GetObjectSize() const noexcept {
    assert(type_ == type::kObject);
    return obj_.size();
  }
  const std::string& Value::GetObjectKey(size_t index) const noexcept {
    assert(type_ == type::kObject);
    return obj_[index].first;
  }
  size_t Value::GetObjectKeyLength(size_t index) const noexcept {
    assert(type_ == type::kObject);
    return obj_[index].first.size();
  }
  const Value& Value::GetObjectValue(size_t index) const noexcept {
    assert(type_ == type::kObject);
    return obj_[index].second;
  }
  void Value::SetObjectValue(const std::string& key, const Value& val) noexcept {
    assert(type_ == type::kObject);
    auto index = FindObject(key);
    if (index >= 0) 
      obj_[index].second = val;
    else 
      obj_.push_back(make_pair(key, val));
  }
  void Value::SetObject(const std::vector<std::pair<std::string, Value>>& obj) noexcept {
    if (type_ == type::kObject) {
      obj_ = obj;
    } else {
      free();
      type_ == type::kObject;
      new(&obj_) std::vector<std::pair<std::string, Value>>(obj);
    }
  }
  long long Value::FindObject(const std::string& key) const noexcept {
    assert(type_ == type::kObject);
    for (int i = 0; i < obj_.size(); ++i) {
      if (obj_[i].first == key) 
        return i;
    }
    return -1;
  }
  void Value::RemoveObject(size_t index) noexcept {
    assert(type_ == type::kObject);
    obj_.erase(obj_.begin() + index, obj_.begin() + index + 1);
  }
  void Value::ClearObject() noexcept {
    assert(type_ == type::kObject);
    obj_.clear();
  }

  void Value::Parse(const std::string& content) {
    Parser(*this, content);
  }

  bool operator==(const Value& lhs, const Value& rhs) noexcept {
    if (lhs.type_ != rhs.type_) 
      return false;

    switch(lhs.type_) {
      case type::kString:
        return lhs.str_ == rhs.str_;
      case type::kNumber:
        return lhs.num_ == rhs.num_;
      case type::kArray:
        return lhs.arr_ == rhs.arr_;
      case type::kObject:
        if (lhs.GetObjectSize() != rhs.GetObjectSize()) 
          return false;
        for (size_t i = 0; i < lhs.GetObjectSize(); ++i) {
          auto index = rhs.FindObject(lhs.GetObjectKey(i));
          if (index < 0 || lhs.GetObjectValue(i) != rhs.GetObjectValue(index)) 
            return false;
        }
        return true;
    }
    return true;
  }
  bool operator!=(const Value& lhs, const Value& rhs) noexcept {
    return !(lhs == rhs);
  }
  
} // namespace json

} // namespace util
} // namespace ctgfs

