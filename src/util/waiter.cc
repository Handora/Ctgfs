/*
 * Authors: Chen Qian(qcdsr970209@gmail.com)
 */

#include <util/waiter.h>

namespace ctgfs {
namespace util {

Waiter::Waiter(uint64_t timeout)
    : timeout_(timeout), completed_(false), status_(Status::OK()) {}

Waiter::~Waiter() {}

Status Waiter::Wait(uint64_t timeout_ms) {
  if (completed_) {
    return status_;
  }
  uint64_t timeout = 0;
  if (timeout_ms != 0) {
    timeout = timeout_ms;
  } else {
    if (timeout_ != 0) {
      timeout = timeout_;
    }
  }

  std::unique_lock<bthread::Mutex> lk(mutex_);
  if (timeout) {
    completed_ = !cond_.wait_for(lk, timeout * 1000);
    if (!completed_) {
      return Status::TimeOut("Waiter timeout for " + std::to_string(timeout) +
                             "ms");
    }
  } else {
    cond_.wait(lk);
  }

  return status_;
}

void Waiter::Signal(Status status) {
  completed_ = true;
  status_ = status;
  cond_.notify_one();
}

}  // namespace util
}  // namespace ctgfs
