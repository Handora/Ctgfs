#pragma once

#include "rpc/marshall.h"
#include "rpc/rpc.h"
#include <memory>

namespace ctgfs {
namespace info_collector {

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
  struct ServerInfo {
    unsigned long long file_num = 0;
    unsigned long long disk_usage = 0;
  };

  /* setter, getter for i_ */
  ServerInfo Get() const;
  void Set(const ServerInfo& atrr);

  /* send extent_server info via rpc */
  void SendHeartBeat(std::string addr) const;

 private:
  /* i_ saves the information of extene_server. */
  ServerInfo i_;
};

/* encode/decode of ServerInfo should implemented. */
marshall& operator<<(marshall &m, const InfoCollector::ServerInfo& i);
unmarshall& operator>>(unmarshall &u, InfoCollector::ServerInfo& i);

}}
