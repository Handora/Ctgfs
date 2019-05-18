// the extent server implementation

#include "fs/extent_server.h"
#include "fs/info_collector.h"
#include "master/master_protocol.h"
#include "master/master.h"
#include "rpc/rpc.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <thread>
#include <memory>
#include <unordered_set>

namespace ctgfs {
namespace server {

using namespace ctgfs::info_collector;

extent_server::extent_server() {
  VERIFY(pthread_mutex_init(&server_mu_, 0) == 0);
  int res;
  VERIFY(put(0x00000001, "", res) == extent_protocol::OK);

  InfoCollector* collector = InfoCollector::GetInstance();

  collector->SendHeartBeat("1234");
}

extent_server::~extent_server() {}

int extent_server::put(extent_protocol::extentid_t id, std::string buf, int &)
{
  ScopedLock lm(&server_mu_);
  std::map<extent_protocol::extentid_t, extent*>::iterator it = extent_map_.find(id);
  unsigned int now = (unsigned int)time(NULL);
  InfoCollector* collector = InfoCollector::GetInstance();
  if (it != extent_map_.end()) {
    /* update the extent_server info for the file update. */
    collector->AddDirtyData(id, buf.size(), 2);

    it->second->ctime = now;
    it->second->mtime = now;
    it->second->content = buf;

    return extent_protocol::OK;
  }
  
  extent *value = new extent(now, now, now, buf);
  extent_map_.insert(std::pair<extent_protocol::extentid_t, extent*>(id, value));

  /* update the extent_server info for the file insertion. */
  collector->AddDirtyData(id, value->content.size(), 1);

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

    /* update the extent_server info for the file size change. */
    InfoCollector* collector = InfoCollector::GetInstance();
    collector->AddDirtyData(id, new_size, 2);

    return extent_protocol::OK;
  }

  return extent_protocol::NOENT;
}

int extent_server::remove(extent_protocol::extentid_t id, int &)
{
  ScopedLock lm(&server_mu_);
  std::map<extent_protocol::extentid_t, extent*>::iterator it = extent_map_.find(id);
  if (it != extent_map_.end()) {
    delete it->second;
    extent_map_.erase(it);

    /* update the extent_server info for the file delete. */
    InfoCollector* collector = InfoCollector::GetInstance();
    collector->AddDirtyData(id, 0, 0);

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
    ret = ptr_rpc_cl->call(extent_protocol::put, extent.first, std::move((extent.second)->content), r);

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
  
}}
