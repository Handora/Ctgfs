#pragma once

#ifndef __INFO_DETECTOR_H
#define __INFO_DETECTOR_H

struct fs_info {
  unsigned int file_num = 0;
  unsigned int disk_usage = 0;
};

class InfoDetector {
 public:
  static InfoDetector* detector();

  ~InfoDetector() {
    delete detector_;
    detector_ = nullptr;

  }

 private:
  static InfoDetector* detector_;

  InfoDetector() { }
  InfoDetector(const InfoDetector&);
  InfoDetector& operator=(const InfoDetector&);

 public:
  friend class extent_server;

  fs_info get() const;

 private:
  void set(const fs_info& atrr);
  fs_info attr;
};

#endif
