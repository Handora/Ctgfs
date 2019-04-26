#include "info_detector.h"

/* initialze the instance */
InfoDetector* InfoDetector::detector_ = new InfoDetector();

InfoDetector* InfoDetector::detector() {
  return detector_;
}

fs_info InfoDetector::get() const {
  return this->attr;
}

void InfoDetector::set(const fs_info& atrr) {
  this->attr = attr;
}