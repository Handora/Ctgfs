/*
 * author: wfgu(peter.wfgu@gmail.com)
 * Based on miloyip/json-tutorial(https://github.com/miloyip/json-tutorial)
 * and Syopain/Json(https://github.com/Syopain/Json)
 * */

#include <util/jsonValue.h>

namespace ctgfs {
namespace util {

namespace json {

class Generator final {
 public:
  Generator(const Value& val, std::string& result);

 private:
  void stringifyValue(const Value& v);
  void stringifyString(const std::string& str);

  std::string& res_;
};

} // namespace json

} // namespace ctgfs
} // namespace util
