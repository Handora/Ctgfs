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

class Parser final {
 public:
  Parser(Value& val, const std::string& content);

  Status Init();
 private:
  void parseWhitespace() noexcept;
  void parseValue();
  void parseLiteral(const char* literal, type t);
  void parseNumber();
  void parseString();
  void parseStringRaw(std::string& tmp);
  void parseHex4(const char*& p, unsigned &u);
  void parseEncodeUTF8(std::string& s, unsigned u);
  void parseArray();
  void parseObject();

 private:
  Value& val_;
  const char* cur_;
};

} // namespace json


} // namespace ctgfs
} // namespace util
