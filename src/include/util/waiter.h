/*
 * Authors: Chen Qian(qcdsr970209@gmail.com)
 */

#pragma once

#include <memory>

#include <bthread/condition_variable.h>
#include <util/status.h>

namespace ctgfs {
namespace util {

class Waiter {
 public:
  explicit Waiter(uint64_t timeout = 0);
  ~Waiter();

  Status Wait(uint64_t timeout_ms = 0);
  void Signal(Status status = Status::OK());

 private:
  uint64_t timeout_;
  bool completed_;
  Status status_;
  bthread::ConditionVariable cond_;
  bthread::Mutex mutex_;
};

} // namespace util
} // namespace ctgfs
