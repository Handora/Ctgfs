#include "info_collector.h"

namespace ctgfs {
namespace info_collector {

/* initialze the instance */
InfoCollector* InfoCollector::collector_ = new InfoCollector();

InfoCollector* InfoCollector::GetInstance() {
  return collector_;
}

Info InfoCollector::Get() const {
  return info;
}

void InfoCollector::Set(const Info& i) {
  info = i;
}

}}