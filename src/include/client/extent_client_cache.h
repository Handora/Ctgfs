// this is the extent cache

#ifndef extent_client_cache_h
#define extent_client_cache_h

#include <string>
#include <map>
#include "fs/extent_protocol.h"
#include "client/extent_client.h"
#include "client/lock_client_cache.h"
#include <client/client.h>

using namespace ::ctgfs::client;
using namespace ::ctgfs::server;

class extent_client_cache : public extent_client {

 public:
  struct extent {
    unsigned int atime;
    unsigned int mtime;
    unsigned int ctime;
    std::string content;
    bool deleted;
    bool dirty;

    extent(unsigned int a, unsigned int m,
           unsigned int c, std::string v)
      : atime(a), mtime(m), ctime(c), content(v),
        deleted(false), dirty(false) {}
  };

  extent_client_cache(std::string dst, Client* client);
  ~extent_client_cache();

  extent_protocol::status put(extent_protocol::extentid_t id, std::string);
  extent_protocol::status get(extent_protocol::extentid_t id, std::string &);
  extent_protocol::status getattr(extent_protocol::extentid_t id, extent_protocol::attr &);
  extent_protocol::status remove(extent_protocol::extentid_t id);
  extent_protocol::status setattr(extent_protocol::extentid_t id, extent_protocol::attr &);
  extent_protocol::status flush(extent_protocol::extentid_t id);

 private:
  pthread_mutex_t cache_mu_;
  std::map<extent_protocol::extentid_t, extent*> cache_map_; 
  Client* client_;
  void retry(extent_protocol::extentid_t id);
  extent_protocol::status callGet(extent_protocol::extentid_t id, std::string& buf);
  extent_protocol::status callGetAttr(extent_protocol::extentid_t id, extent_protocol::attr& attr);
  extent_protocol::status callRemove(extent_protocol::extentid_t id, int& r); 
  extent_protocol::status callPut(extent_protocol::extentid_t id, std::string&, int& r);
  extent_protocol::status callSetAttr(extent_protocol::extentid_t id, extent_protocol::attr& attr, int& r);
};

#endif 
