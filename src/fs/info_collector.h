#pragma once

#ifndef __INFO_COLLECTOR_H
#define __INFO_COLLECTOR_H

namespace ctgfs {
namespace info_collector {

struct Info {
  unsigned int file_num = 0;
  unsigned int disk_usage = 0;
};

class InfoCollector {
 public:
  static InfoCollector* GetInstance();

  ~InfoCollector() {
    delete collector_;
    collector_ = nullptr;
  }


 private:
  static InfoCollector* collector_;

  InfoCollector() { }
  InfoCollector(const InfoCollector&);
  InfoCollector& operator=(const InfoCollector&);

 public:
  friend class extent_server;

  Info Get() const;

 private:
  void Set(const Info&);

  Info info;
};
}}


#endif
