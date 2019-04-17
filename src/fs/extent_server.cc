// the extent server implementation

#include "extent_server.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extent_server::extent_server() {
  VERIFY(pthread_mutex_init(&server_mu_, 0) == 0);
  int res;
  VERIFY(put(0x00000001, "", res) == extent_protocol::OK);
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

    return extent_protocol::OK;
  }

  return extent_protocol::OK;
}

