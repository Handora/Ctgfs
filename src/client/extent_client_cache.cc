// the extent cache implementation

#include "client/extent_client_cache.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extent_client_cache::extent_client_cache(std::string dst, Client* client)
  : extent_client(dst), client_(client) {
  VERIFY(pthread_mutex_init(&cache_mu_, 0) == 0);
}

extent_client_cache::~extent_client_cache() {}

extent_protocol::status extent_client_cache::put(extent_protocol::extentid_t id, std::string buf)
{
  pthread_mutex_lock(&cache_mu_);

  std::map<extent_protocol::extentid_t, extent*>::iterator it = cache_map_.find(id);
  extent *value = NULL;
  unsigned int now = (unsigned int)time(NULL);
  if (it != cache_map_.end()) {
    value = it->second;
  } else {
    // BUG: Need get the attribute(atime) from server
    // I don't want to handle it too (~0v0)~
    value = new extent(now, now, now, buf);
    cache_map_.insert(std::pair<extent_protocol::extentid_t, extent*>(id, value));
  }

  if (value->deleted)
    value->atime = now;
  value->ctime = now;
  value->mtime = now;
  value->content = buf;
  value->dirty = true;
  value->deleted = false;

  pthread_mutex_unlock(&cache_mu_);
  return extent_protocol::OK;
}

extent_protocol::status extent_client_cache::get(extent_protocol::extentid_t id, std::string &buf)
{
  pthread_mutex_lock(&cache_mu_);
  extent_protocol::status ret = extent_protocol::OK;

  std::map<extent_protocol::extentid_t, extent*>::iterator it = cache_map_.find(id);
  unsigned int now = time(NULL);
  extent_protocol::attr attr;
  if (it == cache_map_.end()) {
    pthread_mutex_unlock(&cache_mu_);

    // ret = cl->call(extent_protocol::get, id, buf);
    ret = callGet(id, buf);
    if (ret != extent_protocol::OK) 
        return ret;

    // ret = cl->call(extent_protocol::getattr, id, attr);
    ret = callGetAttr(id, attr);
    if (ret != extent_protocol::OK) 
      return ret;

    pthread_mutex_lock(&cache_mu_);
    extent* value = new extent(attr.atime, attr.mtime,
                               attr.ctime, buf);
    
    cache_map_.insert(std::pair<extent_protocol::extentid_t, extent*>(id, value));
  } else {
    if (it->second->deleted) {
      pthread_mutex_unlock(&cache_mu_);
      return extent_protocol::NOENT;
    }
    buf = it->second->content;
    it->second->atime = now;
    it->second->dirty = true;
  }
  
  pthread_mutex_unlock(&cache_mu_);
  return extent_protocol::OK;
}

extent_protocol::status extent_client_cache::getattr(extent_protocol::extentid_t id, extent_protocol::attr &attr)
{
  pthread_mutex_lock(&cache_mu_);
  extent_protocol::status ret = extent_protocol::OK;

  std::map<extent_protocol::extentid_t, extent*>::iterator it = cache_map_.find(id);
  std::string buf;
  if (it == cache_map_.end()) {
    pthread_mutex_unlock(&cache_mu_);

    // ret = cl->call(extent_protocol::getattr, id, attr);
    ret = callGetAttr(id, attr);
    if (ret != extent_protocol::OK)
      return ret;

    // ret = cl->call(extent_protocol::get, id, buf);
    ret = callGet(id, buf);
    if (ret != extent_protocol::OK)
      return ret;

    pthread_mutex_lock(&cache_mu_);
    extent* value = new extent(attr.atime, attr.mtime, attr.ctime, buf);
    
    cache_map_.insert(std::pair<extent_protocol::extentid_t, extent*>(id, value));
  } else {
    if (it->second->deleted) {
      pthread_mutex_unlock(&cache_mu_);
      return extent_protocol::NOENT;
    }
    attr.ctime = it->second->ctime;
    attr.atime = it->second->atime;
    attr.mtime = it->second->mtime;
    attr.size = it->second->content.size();
  }
  
  pthread_mutex_unlock(&cache_mu_);
  return extent_protocol::OK;
}

extent_protocol::status extent_client_cache::setattr(extent_protocol::extentid_t id, extent_protocol::attr& attr)
{
  pthread_mutex_lock(&cache_mu_);
  extent_protocol::status ret = extent_protocol::OK;

  std::map<extent_protocol::extentid_t, extent*>::iterator it = cache_map_.find(id);
  extent *value = NULL;
  std::string buf;
  if (it != cache_map_.end()) {
    if (it->second->deleted) {
      pthread_mutex_unlock(&cache_mu_);
      return extent_protocol::NOENT;
    }
    value = it->second;
  } else {
    pthread_mutex_unlock(&cache_mu_);

    // ret = cl->call(extent_protocol::get, id, buf);
    ret = callGet(id, buf);
    if (ret != extent_protocol::OK) {
      return ret;
    }

    pthread_mutex_lock(&cache_mu_);

    value = new extent(attr.atime, attr.ctime, attr.mtime, buf);
    cache_map_.insert(std::pair<extent_protocol::extentid_t, extent*>(id, value));
  }

  size_t old_size = value->content.size();
  size_t new_size = attr.size;
  if (old_size != new_size) {
    if (new_size > old_size) {
      value->content += std::string(new_size - old_size, '\0');
    } else {
      value->content.resize(new_size);
    }
  }

  value->atime = attr.atime;
  value->ctime = attr.ctime;
  value->mtime = attr.mtime;
  value->dirty = true;

  pthread_mutex_unlock(&cache_mu_);
  return extent_protocol::OK;
}

extent_protocol::status extent_client_cache::remove(extent_protocol::extentid_t id)
{
  ScopedLock lm(&cache_mu_);
  std::map<extent_protocol::extentid_t, extent*>::iterator it = cache_map_.find(id);
  extent *value = NULL;

  if (it != cache_map_.end()) {
    it->second->deleted = true;
    it->second->dirty = true;

    return extent_protocol::OK;
  } else {
    value = new extent(0, 0, 0, "");
    value->deleted = true;
    value->dirty = true;
    cache_map_.insert(std::pair<extent_protocol::extentid_t, extent*>(id, value));
    return extent_protocol::OK;
  }
}

extent_protocol::status extent_client_cache::flush(extent_protocol::extentid_t id) {
  pthread_mutex_lock(&cache_mu_);
  
  extent_protocol::status ret = extent_protocol::OK;
  int r;
  std::map<extent_protocol::extentid_t, extent*>::iterator it = cache_map_.find(id);

  if (it != cache_map_.end()) {
    extent_protocol::extentid_t id = it->first;
    extent* value = it->second;
    if (value->dirty) {
      if (value->deleted) {
        pthread_mutex_unlock(&cache_mu_);

        // ret = cl->call(extent_protocol::remove, id, r);
        ret = callRemove(id, r);
        if (ret != extent_protocol::OK && ret != extent_protocol::NOENT)
          return ret;

        pthread_mutex_lock(&cache_mu_);
      } else {
        pthread_mutex_unlock(&cache_mu_);

        // ret = cl->call(extent_protocol::put, id, value->content, r);
        ret = callPut(id, value->content, r);
        if (ret != extent_protocol::OK && ret != extent_protocol::NOENT)
          return ret;

        extent_protocol::attr attr;
        attr.size = value->content.size();
        attr.atime = value->atime;
        attr.mtime = value->mtime;
        attr.ctime = value->ctime;

        int r;
        // ret = cl->call(extent_protocol::setattr, id, attr, r);
        ret = callSetAttr(id, attr, r);
        if (ret != extent_protocol::OK && ret != extent_protocol::NOENT)
          return ret;
        pthread_mutex_lock(&cache_mu_);
      }
    }
  }

  cache_map_.erase(id);
  pthread_mutex_unlock(&cache_mu_);
  return ret;
}

void extent_client_cache::retry(extent_protocol::extentid_t id) {
  client_->FailCache(id);
  const auto& addr = client_->GetKVAddrByInum(id);
  ConnectTo(addr);
}

extent_protocol::status extent_client_cache::callGet(extent_protocol::extentid_t id, std::string& buf) {
  auto ret = cl->call(extent_protocol::get, id, buf);
  if (ret != extent_protocol::OK) {
    retry(id);
    // ? need clear?
    buf.clear();
    ret = cl->call(extent_protocol::get, id, buf);
  }
  return ret;
}

extent_protocol::status extent_client_cache::callGetAttr(extent_protocol::extentid_t id, extent_protocol::attr& attr) {
  auto ret = cl->call(extent_protocol::getattr, id, attr);
  if(ret != extent_protocol::OK) {
    retry(id);
    ret = cl->call(extent_protocol::getattr, id, attr);
  }
  return ret;
}

extent_protocol::status extent_client_cache::callRemove(extent_protocol::extentid_t id, int& r) {
  auto ret = cl->call(extent_protocol::remove, id, r);
  if (ret != extent_protocol::OK && ret != extent_protocol::NOENT) {
    retry(id);
    ret = cl->call(extent_protocol::remove, id, r);
  }
  return ret;
}

extent_protocol::status extent_client_cache::callPut(extent_protocol::extentid_t id, std::string& content, int& r) {
  retry(id);
  auto ret = cl->call(extent_protocol::put, id, content, r);
  if (ret != extent_protocol::OK && ret != extent_protocol::NOENT) {
    retry(id);
    ret = cl->call(extent_protocol::put, id, content, r);
  }
  return ret;
}

extent_protocol::status extent_client_cache::callSetAttr(extent_protocol::extentid_t id, extent_protocol::attr& attr, int& r) {
  retry(id);
  auto ret = cl->call(extent_protocol::setattr, id, attr, r);
  if (ret != extent_protocol::OK && ret != extent_protocol::NOENT) {
    retry(id);
    ret = cl->call(extent_protocol::setattr, id, attr, r);
  }
  return ret;
}
