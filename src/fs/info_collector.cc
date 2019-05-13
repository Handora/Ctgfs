#include "fs/info_collector.h"
#include "master/master_protocol.h"
#include <mutex>
#include <thread>

namespace ctgfs {
namespace info_collector {

/* initialze the instance */
InfoCollector* InfoCollector::instance_ = new InfoCollector();

InfoCollector* InfoCollector::GetInstance() {
  return instance_;
}

/* Get, Set should be a thread-safe operation. */
std::mutex info_mu;

InfoCollector::ServerInfo InfoCollector::Get() const {
  std::lock_guard<std::mutex> locker(info_mu);
  return i_;
}

void InfoCollector::Set(const InfoCollector::ServerInfo& i) {
  std::lock_guard<std::mutex> locker(info_mu);
  i_ = i;
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
      /* do this loop every 3s. */
      std::this_thread::sleep_for(std::chrono::seconds(3));
    }

  });

  /* detach this thread for it should run till the main thread is off. */
  t.detach();
}

/* Implement marshall, unmarshall for HeartBeatInfo */
/* type : int, addr: string */ 
/* file_num: uint64 , disk_usage: uint64 */
marshall& operator<<(marshall &m, const InfoCollector::ServerInfo& i) {
  m << i.file_num;
  m << i.disk_usage;
  return m;
}

unmarshall& operator>>(unmarshall &u, InfoCollector::ServerInfo& i) {
  u >> i.file_num;
  u >> i.disk_usage;
  return u;
}

}}
