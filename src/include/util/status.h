/*
 * Authors: Chen Qian(qcdsr970209@gmail.com)
 */

#pragma once

namespace ctgfs {
namespace util {

// utility class that shows the return status of the
// function, OK() indicate the success else you can use
// status.ToString() to get the error message
//
// Example:
// Status s = YourFunctions();
// if (!s.IsOK()) {
//   std::string err = s.ToString();
//   // do for error
// }
// // do for success
class Status {
  enum class Code;

 public:
  Status() : 
    code_(Code::kNull), error_text_("null") {}

  Status(Code code, std::string msg="") : 
      code_(code), error_text_(msg) {}   

  std::string ToString() const {
    return error_text_;
  }

  std::string ErrorText() const {
    return error_text_;
  }

  /*************************** General Part *******************************/
  static Status Null() {
    return Status(Code::kNull);
  }

  bool IsNull() const {
    return (code_ == Code::kNull);
  }

  static Status OK() {
    return Status(Code::kOk);
  }

  bool IsOK() const {
    return (code_ == Code::kOk);
  }

  static Status NotFound(const std::string& msg) {
    return Status(Code::kNotFound, msg);
  }

  bool IsNotFound() const {
    return (code_ == Code::kNotFound);
  }

  static Status TimeOut(const std::string& msg) {
    return Status(Code::kTimeOut, msg);
  }

  bool IsTimeOut() const {
    return (code_ == Code::kTimeOut);
  }

  static Status NotSupported(const std::string& msg) {
    return Status(Code::kNotSupported, msg);
  }

  bool IsNotSupported() const {
    return (code_ == Code::kNotSupported);
  }

  static Status InvalidArgument(const std::string& msg) {
    return Status(Code::kInvalidArgument, msg);
  }

  bool IsInvalidArgument() const {
    return (code_ == Code::kInvalidArgument);
  }

  /*************************** KV Part *******************************/
  static Status NotLeader(const std::string& msg) {
    return Status(Code::kNotLeader, msg);
  }

  bool IsNotLeader() const {
    return (code_ == Code::kNotLeader);
  }

  // --------------------------JSON Part------------------------------//
  static Status ExpectValue() {
    return Status(Code::kExpectValue);
  }

  bool isExpectValue() const {
    return (code_ == Code::kExpectValue);
  }

 private:
  enum class Code {
    // General part 
    kNull,
    kOk,
    kNotFound,
    kTimeOut,
    kNotSupported,
    kInvalidArgument,

    // KV part
    kNotLeader,

    // JSON part
    kExpectValue,
  }; 

  Code code_;
  std::string error_text_;
};

} // namespace util
} // namespace ctgfs
