#include "fs/info_collector.h"
#include "master/master_protocol.h"
#include <mutex>
#include <thread>
#include <map>
#include <algorithm>

namespace ctgfs {
namespace info_collector {

/* initialze the instance */
InfoCollector* InfoCollector::instance_ = new InfoCollector();

InfoCollector* InfoCollector::GetInstance() {
  return instance_;
}

/* Get, Set should be a thread-safe operation. */
std::mutex info_mu;

InfoCollector::ServerInfo InfoCollector::Get() {
  std::lock_guard<std::mutex> locker(info_mu);

  std::map< unsigned long long, std::pair<unsigned long long, int> > m;
  for (int i = 0; i < (int)i_.inum.size(); ++i) {
    m.insert(std::make_pair(i_.inum[i], std::make_pair(i_.sz[i], i_.type[i])));
  }

  std::vector< unsigned long long > temp_inum, temp_sz;
  std::vector< int> temp_type;

  for (auto p : m) {
    temp_inum.push_back(p.first);
    temp_sz.push_back(p.second.first);
    temp_type.push_back(p.second.second);
  }

  i_.inum = std::move(temp_inum);
  i_.sz = std::move(temp_sz);
  i_.type = std::move(temp_type);
  return i_;
}

void InfoCollector::AddDirtyData(unsigned long long inum, unsigned long long size, int type) {
  std::lock_guard<std::mutex> locker(info_mu);
  i_.inum.push_back(inum);
  i_.sz.push_back(size);
  i_.type.push_back(type);
}

void InfoCollector::Set(const InfoCollector::KVInfo& i) {
  std::lock_guard<std::mutex> locker(info_mu);
  kv_addr_ = i.addr;
  limit_sz_ = i.sz;
}

void InfoCollector::SendHeartBeat(std::string addr) const {
  std::thread t([=]() {

    sockaddr_in ms_sin;
    make_sockaddr(addr.c_str(), &ms_sin);
    std::unique_ptr<rpcc> cl(new rpcc(ms_sin));
    if (cl->bind() != 0) {
      printf("Failed to bind the Heart beat rpc client.\n");
      return;
    }

    while (true) {
      /* send heart beat to master */
      int r;
      cl->call(master_protocol::heart_beat, instance_->Get(), r);

      /* clear the dirty collection. */
      instance_->i_.clear();

      /* do this loop every 3s. */
      std::this_thread::sleep_for(std::chrono::seconds(3));
    }

  });

  /* detach this thread for it should run till the main thread is off. */
  t.detach();
}

void InfoCollector::Regist(const std::string& addr) {
  sockaddr_in ms_sin;
  make_sockaddr(addr.c_str(), &ms_sin);
  std::unique_ptr<rpcc> cl(new rpcc(ms_sin));
  if(cl->bind() != 0) {
    printf("Failed to bind the regist client\n");
    return;
  }
  int r = 0;
  cl->call(master_protocol::regist, kv_addr_, limit_sz_, r);
}

/* Implement marshall, unmarshall for HeartBeatInfo */
/* type : int, addr: string */ 
/* file_num: uint64 , disk_usage: uint64 */
marshall& operator<<(marshall &m, const InfoCollector::ServerInfo& i) {
  m << i.inum;
  m << i.sz;
  m << i.type;
  return m;
}

unmarshall& operator>>(unmarshall &u, InfoCollector::ServerInfo& i) {
  u >> i.inum;
  u >> i.sz;
  u >> i.type;
  return u;
}

marshall& operator<<(marshall& m, const InfoCollector::KVInfo& i) {
  m << i.addr;
  m << i.sz;
  return m;
}

unmarshall& operator>>(unmarshall &u, InfoCollector::KVInfo& i) {
  u >> i.addr;
  u >> i.sz;
  return u;
}

}}
