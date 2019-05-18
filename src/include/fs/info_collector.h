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
  InfoCollector() {  }
  InfoCollector(const InfoCollector&);
  InfoCollector& operator=(const InfoCollector&);

  static InfoCollector* instance_;

 public:
  struct ServerInfo {
    std::vector< unsigned long long > inum, sz;
    std::vector< int> type;        /* 0: delete */
                                   /* 1: add */
                                   /* 2: update */
    void clear() {
      inum.clear();
      sz.clear();
      type.clear();
    }
  };

  struct KVInfo{
    std::string addr;
    unsigned long long sz;
  };

  /* setter, getter for i_ */
  ServerInfo Get();
  void AddDirtyData(unsigned long long, unsigned long long, int);
  
  void Set(const KVInfo& info);
  /* send extent_server info via rpc */
  void SendHeartBeat(std::string addr) const;
  
  void Regist(const std::string& addr);

 private:
  /* i_ saves the information of extene_server. */
  ServerInfo i_;
  std::string kv_addr_;
  unsigned long long limit_sz_;
};

/* encode/decode of ServerInfo should implemented. */
marshall& operator<<(marshall &m, const InfoCollector::ServerInfo& i);
unmarshall& operator>>(unmarshall &u, InfoCollector::ServerInfo& i);
marshall& operator<<(marshall &m, const InfoCollector::KVInfo& i);
unmarshall& operator>>(unmarshall &u, InfoCollector::KVInfo& i);

}}
