#pragma once

namespace ctgfs {
namespace info_collector {

struct ServerInfo {
  unsigned long long file_num = 0;
  unsigned long long disk_usage = 0;
  // unsigned long long disk_volume = 0;
};

class InfoCollector {
 public:
  static InfoCollector* GetInstance();

  ~InfoCollector() {
    delete instance_;
    instance_ = nullptr;
  }

 private:
  InfoCollector() { i_.file_num = i_.disk_usage = 0; }
  InfoCollector(const InfoCollector&);
  InfoCollector& operator=(const InfoCollector&);

  static InfoCollector* instance_;

 public:
  /* setter, getter for i_ */
  ServerInfo Get() const;
  void Set(const ServerInfo& atrr);

 private:
  /* i_ saves the information of extene_server. */
  ServerInfo i_;
};

}}