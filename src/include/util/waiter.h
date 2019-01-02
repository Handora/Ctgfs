/*
 * Authors: Chen Qian(qcdsr970209@gmail.com)
 */

#pragma once

#include <memory>

#include <bthread/condition_variable.h>
#include <util/status.h>

namespace ctgfs {
namespace util {

// waiter interface for synchronization or
// asynchronization programming
//
// Example:
// std::shared_ptr<Waiter> waiter = std::make_shared<Waiter>();
// doAsynProcessFunc(waiter);
//
// Status s = waiter->Wait(1000);
// if (!s.ok()) {
//   // do for error
// }
class Waiter {
 public:
  explicit Waiter(uint64_t timeout = 0);
  ~Waiter();

  // wait with timeous as milisecond
  Status Wait(uint64_t timeout_ms = 0);

  // signal with status
  void Signal(Status status = Status::OK());

 private:
  // wait time before timeout
  uint64_t timeout_;
  bool completed_;
  Status status_;

  // synchronizaion variables
  bthread::ConditionVariable cond_;
  bthread::Mutex mutex_;
};

}  // namespace util
}  // namespace ctgfs
