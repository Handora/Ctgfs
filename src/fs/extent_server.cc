// the extent server implementation

#include "fs/extent_server.h"
#include "fs/info_detector.h"
#include "fs/heart_beat_sender.h"
#include "rpc/rpc.h"
#include "master.pb.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <thread>
#include <memory>
#include <unordered_set>
// #include <brpc/server.h>
// #include <brpc/stream.h>
// #include <brpc/channel.h>

extent_server::extent_server() {
  VERIFY(pthread_mutex_init(&server_mu_, 0) == 0);
  int res;
  VERIFY(put(0x00000001, "", res) == extent_protocol::OK);

  
  /* the instance to get the file system info, it likes a global pointer. */
 // InfoDetector* info = InfoDetector::detector();

 // /* a shared_ptr to the HeartBeatInfo. */
 // auto p_heart_beat_info = std::make_shared<ctgfs::heart_beat::HeartBeatInfo>();

 // std::string addr = std::string("127.0.0.1:1235");

 // /* Sender is used to send the heart beat package. */
 // ctgfs::heart_beat::HeartBeatSender sender(addr, p_heart_beat_info);
 // std::thread t_heart_beat([&]() {
 //   while (true) {
 //     auto p_heart_beat_info = std::make_shared<ctgfs::heart_beat::HeartBeatInfo>(
 //       (ctgfs::heart_beat::HeartBeatInfo){
 //         ctgfs::HeartBeatMessageRequest_HeartBeatType::HeartBeatMessageRequest_HeartBeatType_kInfoUpdate,
 //         std::string("127.0.0.1:1235"),
 //         info->get().file_num, 
 //         info->get().disk_usage
 //       });
 //     sender.SetHeartBeatInfo(p_heart_beat_info);
 //     sender.SendHeartBeat();
 //     std::this_thread::sleep_for(std::chrono::seconds(3));
 //   }
 // });

 // /* detach this thread for it should run till the main thread is off. */
 // t_heart_beat.detach();
}

extent_server::~extent_server() {}

int extent_server::put(extent_protocol::extentid_t id, std::string buf, int &)
{
  ScopedLock lm(&server_mu_);
  std::map<extent_protocol::extentid_t, extent*>::iterator it = extent_map_.find(id);
  unsigned int now = (unsigned int)time(NULL);
  if (it != extent_map_.end()) {
    it->second->ctime = now;
    it->second->mtime = now;
    it->second->content = buf;

    return extent_protocol::OK;
  }
  
  extent *value = new extent(now, now, now, buf);
  extent_map_.insert(std::pair<extent_protocol::extentid_t, extent*>(id, value));

  // InfoDetector* info = InfoDetector::detector();
  // fs_info attr = info->get();
  // attr.file_num++;
  // attr.disk_usage += buf.size();
  // info->set(attr);

  return extent_protocol::OK;
}

int extent_server::get(extent_protocol::extentid_t id, std::string &buf)
{
  ScopedLock lm(&server_mu_);
  std::map<extent_protocol::extentid_t, extent*>::iterator it = extent_map_.find(id);
  unsigned int now = time(NULL);
  if (it != extent_map_.end()) {
    it->second->atime = now;
    buf = it->second->content;

    return extent_protocol::OK;
  }
  
  return extent_protocol::NOENT;
}

int extent_server::getattr(extent_protocol::extentid_t id, extent_protocol::attr &a)
{
  ScopedLock lm(&server_mu_);
  std::map<extent_protocol::extentid_t, extent*>::iterator it = extent_map_.find(id);
  if (it != extent_map_.end()) {
    a.size = it->second->content.size();
    a.atime = it->second->atime;
    a.mtime = it->second->mtime;
    a.ctime = it->second->ctime;

    return extent_protocol::OK;
  }

  return extent_protocol::NOENT;
}

int extent_server::setattr(extent_protocol::extentid_t id, extent_protocol::attr a, int& r)
{
  ScopedLock lm(&server_mu_);
  std::map<extent_protocol::extentid_t, extent*>::iterator it = extent_map_.find(id);
  if (it != extent_map_.end()) {
    size_t old_size = it->second->content.size();
    size_t new_size = a.size;
    if (old_size == new_size) {
      return extent_protocol::OK;
    }
    unsigned int now = (unsigned int)time(NULL);
    it->second->mtime = now;
    it->second->ctime = now;
    if (new_size > old_size) {
      it->second->content += std::string(new_size - old_size, '\0');
    } else {
      it->second->content.resize(new_size);
    }

    // InfoDetector* info = InfoDetector::detector();
    // fs_info attr = info->get();
    // attr.disk_usage += new_size - old_size;
    // info->set(attr);

    return extent_protocol::OK;
  }

  return extent_protocol::NOENT;
}

int extent_server::remove(extent_protocol::extentid_t id, int &)
{
  ScopedLock lm(&server_mu_);
  std::map<extent_protocol::extentid_t, extent*>::iterator it = extent_map_.find(id);
  if (it != extent_map_.end()) {
    int file_size = it->second->content.size();

    delete it->second;
    extent_map_.erase(it);

    // InfoDetector* info = InfoDetector::detector();
    // fs_info attr = info->get();
    // attr.file_num--;
    // attr.disk_usage -= file_size;
    // info->set(attr);

    return extent_protocol::OK;
  }

  return extent_protocol::OK;
}


int extent_server::move(std::vector<extent_protocol::extentid_t> ids, std::string dst, int&)
{
  std::map<extent_protocol::extentid_t, extent*> extents;
  /* Since we cannot lock during rpc, we should take extents out. */
  {
    ScopedLock lm_1(&server_mu_);
    for (extent_protocol::extentid_t eid : ids) {
      std::map<extent_protocol::extentid_t, extent*>::iterator it = extent_map_.find(eid);
      if (it != extent_map_.end()) {
        extents.insert(*it);
      }
    }
  }

  /* parse the dst, ip:port */
  size_t pos_ = dst.find_last_of(":");
  if (pos_ == std::string::npos) {
    /* the dst format is wrong. */
    return extent_protocol::IOERR;
  }
  std::string dst_ip = dst.substr(0, pos_);
  std::string dst_port = dst.substr(pos_ + 1);

  /* Create a rpc client to connect the dst. */
  sockaddr_in dst_sin;
  make_sockaddr(dst_port.c_str(), &dst_sin);
  std::unique_ptr<rpcc> ptr_rpc_cl(new rpcc(dst_sin));
  if (ptr_rpc_cl->bind() != 0) {
    printf("Move failed: failed to bind the rpc client.\n");
    return extent_protocol::RPCERR;
  }

  std::unordered_set<extent_protocol::extentid_t> deleted_ok;
  /* to put extents to the target server via rpc. */
  for (const std::pair<extent_protocol::extentid_t, extent*> extent : extents) {
    extent_protocol::status ret = extent_protocol::OK;
    int r;
    ret = ptr_rpc_cl->call(extent_protocol::put, extent.first, (extent.second)->content, r);

    /* if it does need to check the status of r here? */
    
    if (ret == extent_protocol::OK) {
      /* move succeeded, mark this extent deleted. */
      deleted_ok.insert(extent.first);
    }
  }

  /* remove those extents which were moved successfully from current server. */
  {
    ScopedLock lm_2(&server_mu_);
    for (const std::pair<extent_protocol::extentid_t, extent*> extent : extents) {
      if (deleted_ok.find(extent.first) != deleted_ok.end()) {
        delete extent_map_[extent.first];
        extent_map_[extent.first] = nullptr;

        extent_map_.erase(extent.first);
      } 
    }
  } 

  return extent_protocol::OK;
}


