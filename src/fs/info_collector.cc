#include "fs/info_collector.h"
#include <mutex>


namespace ctgfs {
namespace info_collector {

/* initialze the instance */
InfoCollector* InfoCollector::instance_ = new InfoCollector();

InfoCollector* InfoCollector::GetInstance() {
  return instance_;
}

/* Get, Set should be a thread-safe operation. */
std::mutex info_mu;

ServerInfo InfoCollector::Get() const {
  std::lock_guard<std::mutex> locker(info_mu);
  return i_;
}

void InfoCollector::Set(const ServerInfo& i) {
  std::lock_guard<std::mutex> locker(info_mu);
  i_ = i;
}

}}