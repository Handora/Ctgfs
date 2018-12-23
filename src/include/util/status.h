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
  Status(Code code, std::string msg = "") : code_(code), error_text_(msg) {}

  std::string ToString() const { return error_text_; }

  std::string ErrorText() const { return error_text_; }

  /*************************** General Part *******************************/
  static Status OK() { return Status(Code::kOk); }

  bool IsOK() const { return (code_ == Code::kOk); }

  static Status NotFound(const std::string& msg) {
    return Status(Code::kNotFound, msg);
  }

  bool IsNotFound() const { return (code_ == Code::kNotFound); }

  static Status TimeOut(const std::string& msg) {
    return Status(Code::kTimeOut, msg);
  }

  bool IsTimeOut() const { return (code_ == Code::kTimeOut); }

  static Status NotSupported(const std::string& msg) {
    return Status(Code::kNotSupported, msg);
  }

  bool IsNotSupported() const { return (code_ == Code::kNotSupported); }

  static Status InvalidArgument(const std::string& msg) {
    return Status(Code::kInvalidArgument, msg);
  }

  bool IsInvalidArgument() const { return (code_ == Code::kInvalidArgument); }

  static Status Corruption(const std::string& msg) {
    return Status(Code::kCorruption, msg);
  }

  bool IsCorruption() const {
    return (code_ == Code::kCorruption);
  }

  /*************************** KV Part *******************************/
  static Status NotLeader(const std::string& msg) {
    return Status(Code::kNotLeader, msg);
  }

  bool IsNotLeader() const { return (code_ == Code::kNotLeader); }

  /*************************** HeartBeat Part *******************************/
  static Status HeartBeatFail(const std::string& msg = "") {
    return Status(Code::kHeartBeatFail, msg);
  }

  bool IsHeartBeatFail() const { return (code_ == Code::kHeartBeatFail); }

 private:
  enum class Code {
    // General part
    kOk,
    kNotFound,
    kTimeOut,
    kNotSupported,
    kInvalidArgument,
    kCorruption,

    // KV part
    kNotLeader,

    // HeartBeat part
    kHeartBeatFail,
  };

  Code code_;
  std::string error_text_;
};

}  // namespace util
}  // namespace ctgfs
