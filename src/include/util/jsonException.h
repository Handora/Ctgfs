/*
 * author: wfgu(peter.wfgu@gmail.com)
 * Based on miloyip/json-tutorial(https://github.com/miloyip/json-tutorial)
 * and Syopain/Json(https://github.com/Syopain/Json)
 * */

 #include <util/json.h>
 #include <util/jsonValue.h>

namespace ctgfs {
namespace util {

namespace json {
  
class Exception final : public std::logic_error {
 public:
  Exception(const std::string& msg) : logic_error(msg) {  }
}

} // namespace json


} // namespace ctgfs
} // namespace util
